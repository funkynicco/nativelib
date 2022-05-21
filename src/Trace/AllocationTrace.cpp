#include "StdAfx.h"

//!ALLOW_INCLUDE "Windows.h"
//!ALLOW_INCLUDE "DbgHelp.h"

#ifdef NL_PLATFORM_WINDOWS

#include <NativeLib/Trace/AllocationTrace.h>
#include <NativeLib/Helper.h>
#include <NativeLib/Logger.h>

#include <NativeLib/Containers/LinkedStack.h>
#include <NativeLib/Containers/Queue.h>

#include <NativeLib/SystemLayer/SystemLayer.h>
#include <NativeLib/Allocators.h>
#include <NativeLib/Threading/Interlocked.h>

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#endif

#define __DBG_TRACING

namespace nl
{
    namespace trace
    {
        void ResolvePointerData(LONG_PTR ptr, struct PointerInfo* pi);

        enum class IOEvent
        {
            Read,
            Write
        };

        enum class CommandType
        {
            AddAllocation,
            RemoveAllocation,
            PointerInfo
        };

        const char* PipeName = "\\\\.\\pipe\\nl-trace-4661A80D-CD1F-4692-9269-BCC420539E38";

        DWORD WINAPI AllocationTraceThread(LPVOID);

        typedef USHORT(WINAPI* CaptureStackBackTraceType)(ULONG FramesToSkip, ULONG FramesToCapture, PVOID* BackTrace, PULONG BackTraceHash);

        HANDLE g_hThread = nullptr;
        HANDLE g_hIocp = nullptr;
        HANDLE g_hPipe = INVALID_HANDLE_VALUE;
        LARGE_INTEGER g_liFrequency;
        LARGE_INTEGER g_liStart;

        struct
        {
            HANDLE hProcess = nullptr;
            HMODULE hKernel = nullptr;
            CaptureStackBackTraceType CaptureStackBackTracePtr = nullptr;

        } g_callstackCapturing;

        int64_t g_lNextPacketIndex = 0;

#pragma pack(push, 1)
        struct StackInfo
        {
            char Name[64];
        };

        struct FunctionData
        {
            double Time;
            char Filename[MAX_PATH];
            int LineNumber;
            char Function[64];
            ULONGLONG Pointer;
            ULONGLONG SizeOfPointerData;

            ULONGLONG Stack[32];
            USHORT Frames;
        };

        struct PointerInfo
        {
            ULONGLONG Pointer;
            char Function[256];
        };

        struct PointerInfoRequest
        {
            LONGLONG RequestId;
            PointerInfo Info;
        };

        struct PipeCommand
        {
            LONGLONG PacketIndex;
            CommandType Command;

            union
            {
                FunctionData Data;
                ULONGLONG Pointer;
                PointerInfoRequest PointerInfoRequest;
            };
        };
#pragma pack(pop)

        struct OverlappedBuffer
        {
            char Buffer[4096];
            size_t Offset;
            size_t Length;

            SafeLinkedStack<OverlappedBuffer>* Pool;

            OverlappedBuffer* prev;
            OverlappedBuffer* next;
        };

        struct OverlappedEx
        {
            OVERLAPPED Overlapped;
            IOEvent Event;
            OverlappedBuffer* lpBuffer;

            OverlappedEx* next;
        };

        SafeLinkedStack<OverlappedEx> g_overlappedPool;

        void* g_overlappedBuffersMemoryBlock = nullptr;
        SafeLinkedStack<OverlappedBuffer> g_overlappedBufferPool;
        SafeLinkedStack<OverlappedBuffer> g_overlappedBufferPoolPriority;

        SRWLOCK g_sendBufferLock = SRWLOCK_INIT;
        Queue<OverlappedBuffer> g_sendBuffer;
        bool g_bIsSending = false;

        SRWLOCK g_entryLock = SRWLOCK_INIT;

        OverlappedEx* AllocateOverlapped(IOEvent event)
        {
            OverlappedEx* overlapped = nullptr;
            while (!g_overlappedPool.TryPop(&overlapped))
            {
                Sleep(0);
            }

            ZeroMemory(overlapped, sizeof(OverlappedEx));

            overlapped->Event = event;
            return overlapped;
        }

        void FreeOverlapped(OverlappedEx* overlapped)
        {
            g_overlappedPool.Push(overlapped);
        }

        template <bool priority>
        OverlappedBuffer* AllocateOverlappedBuffer()
        {
            SafeLinkedStack<OverlappedBuffer>* pool;
            if constexpr (priority)
                pool = &g_overlappedBufferPoolPriority;
            else
                pool = &g_overlappedBufferPool;

            OverlappedBuffer* lpBuffer;
            while (!pool->TryPop(&lpBuffer))
            {
                Sleep(0);
            }

            lpBuffer->Offset = 0;
            lpBuffer->Length = 0;

            lpBuffer->Pool = pool;

            return lpBuffer;
        }

        void FreeOverlappedBuffer(OverlappedBuffer* lpBuffer)
        {
            lpBuffer->Pool->Push(lpBuffer);
        }

        inline void InitializeCallstackCapturing()
        {
            g_callstackCapturing.hProcess = GetCurrentProcess();

            if (!SymInitialize(g_callstackCapturing.hProcess, nullptr, TRUE))
                throw Exception("Alloc trace setup failed");

            g_callstackCapturing.hKernel = LoadLibrary(L"kernel32.dll");
            if (!g_callstackCapturing.hKernel)
                throw Exception("Alloc trace setup failed `2");

            g_callstackCapturing.CaptureStackBackTracePtr = reinterpret_cast<CaptureStackBackTraceType>(GetProcAddress(
                g_callstackCapturing.hKernel,
                "RtlCaptureStackBackTrace"));

            if (!g_callstackCapturing.CaptureStackBackTracePtr)
                throw Exception("Alloc trace setup failed `3");
        }

        void Setup()
        {
            nl_assert_if_debug(g_hPipe == INVALID_HANDLE_VALUE);

            InitializeCallstackCapturing();

            for (int i = 0; i < 2; ++i)
            {
                g_overlappedPool.Push(nl::memory::ConstructThrow<OverlappedEx>());
            }

            const size_t NumberOfBuffersToAllocate = 16384;
            const size_t NumberOfPriorityBuffersToAllocate = 64;
            g_overlappedBuffersMemoryBlock = nl::systemlayer::GetSystemLayerFunctions()->AllocateVirtualMemory(sizeof(OverlappedBuffer) * (NumberOfBuffersToAllocate + NumberOfPriorityBuffersToAllocate));
            OverlappedBuffer* lpBuffersMemory = static_cast<OverlappedBuffer*>(g_overlappedBuffersMemoryBlock);
            for (size_t i = 0; i < NumberOfBuffersToAllocate; ++i)
            {
                g_overlappedBufferPool.Push(lpBuffersMemory++);
            }

            for (size_t i = 0; i < NumberOfPriorityBuffersToAllocate; ++i)
            {
                g_overlappedBufferPoolPriority.Push(lpBuffersMemory++);
            }

            g_hPipe = CreateFileA(PipeName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
            if (g_hPipe == INVALID_HANDLE_VALUE)
                throw Win32Exception();

            DWORD dwMode = PIPE_READMODE_MESSAGE;
            SetNamedPipeHandleState(g_hPipe, &dwMode, nullptr, nullptr);

            QueryPerformanceFrequency(&g_liFrequency);
            QueryPerformanceCounter(&g_liStart);

            g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);

            CreateIoCompletionPort(g_hPipe, g_hIocp, 0, 0); // associate

            g_hThread = CreateThread(nullptr, 0, AllocationTraceThread, nullptr, 0, nullptr);

            auto overlappedRead = AllocateOverlapped(IOEvent::Read);
            overlappedRead->lpBuffer = AllocateOverlappedBuffer<false>();

            if (!ReadFile(g_hPipe, overlappedRead->lpBuffer->Buffer, ARRAYSIZE(overlappedRead->lpBuffer->Buffer), nullptr, &overlappedRead->Overlapped))
            {
                DWORD dwCode = GetLastError();
                if (dwCode != ERROR_IO_PENDING)
                {
#ifdef __DBG_TRACING
                    char msg[1024];
                    sprintf_s(msg, "err ReadFile code: %u", dwCode);

                    nl::logger::GetGlobalLogger()->Log(
                        nl::LogType::Error,
                        __FILE__,
                        __LINE__,
                        __FUNCTION__,
                        msg);
#endif
                }
            }
        }

        void Terminate()
        {
            nl_assert_if_debug(g_hPipe != INVALID_HANDLE_VALUE);

            PostQueuedCompletionStatus(g_hIocp, 0, 0, nullptr);
            WaitForSingleObject(g_hThread, INFINITE);

            CloseHandle(g_hThread);
            g_hThread = nullptr;

            CloseHandle(g_hIocp);
            g_hIocp = nullptr;

            CloseHandle(g_hPipe);
            g_hPipe = INVALID_HANDLE_VALUE;

            OverlappedEx* overlapped;
            while (g_overlappedPool.TryPop(&overlapped))
            {
                nl::memory::Destroy(overlapped);
            }

            nl::systemlayer::GetSystemLayerFunctions()->FreeVirtualMemory(g_overlappedBuffersMemoryBlock);
        }

        void SendBuffer(OverlappedBuffer* lpBuffer, bool prioritized = false)
        {
            nl_assert_if_debug(lpBuffer->Length != 0);

            AcquireSRWLockExclusive(&g_sendBufferLock);

            if (prioritized)
                g_sendBuffer.AddHead(lpBuffer);
            else
                g_sendBuffer.AddTail(lpBuffer);

            if (!g_bIsSending)
            {
                OverlappedEx* overlapped = AllocateOverlapped(IOEvent::Write);
                overlapped->lpBuffer = g_sendBuffer.PopHead();

                PipeCommand* cmd = reinterpret_cast<PipeCommand*>(overlapped->lpBuffer);
                cmd->PacketIndex = nl::threading::Interlocked::Increment(&g_lNextPacketIndex);

                if (!WriteFile(g_hPipe, overlapped->lpBuffer->Buffer + overlapped->lpBuffer->Offset, (DWORD)overlapped->lpBuffer->Length, nullptr, &overlapped->Overlapped))
                {
                    DWORD dwCode = GetLastError();
                    if (dwCode != ERROR_IO_PENDING)
                    {
                        g_sendBuffer.AddHead(overlapped->lpBuffer);
                        FreeOverlapped(overlapped);

#ifdef __DBG_TRACING
                        char msg[1024];
                        sprintf_s(msg, "err WriteFile code: %u", dwCode);

                        nl::logger::GetGlobalLogger()->Log(
                            nl::LogType::Error,
                            __FILE__,
                            __LINE__,
                            __FUNCTION__,
                            msg);
#endif

                        ReleaseSRWLockExclusive(&g_sendBufferLock);
                        return;
                    }
                }

                g_bIsSending = true;
            }

            ReleaseSRWLockExclusive(&g_sendBufferLock);
        }

        NL_NOINLINE void CaptureCallstack(ULONG skip, char* ptr)
        {
            LPVOID aStack[32];
            auto frames = g_callstackCapturing.CaptureStackBackTracePtr(1 + skip, ARRAYSIZE(aStack), aStack, nullptr);

            char* p = ptr;
            for (int i = (int)frames - 1; i >= 0; --i)
            {
                if (p > ptr)
                    *p++ = '/';

                PointerInfo pi;
                ResolvePointerData(reinterpret_cast<LONG_PTR>(aStack[i]), &pi);
                p += sprintf(p, "%s", pi.Function);
            }

            *p = 0;
        }

        NL_NOINLINE void AddAllocation(const char* filename, int32_t line, const char* function, void* ptr, size_t sizeOfPtrData)
        {
            if (g_hPipe == INVALID_HANDLE_VALUE)
                return;

            //if (!TryAcquireSRWLockExclusive(&g_entryLock))
                //return;

            LARGE_INTEGER li;
            QueryPerformanceCounter(&li);
            double dt = double(li.QuadPart - g_liStart.QuadPart) / g_liFrequency.QuadPart;

            auto lpBuffer = AllocateOverlappedBuffer<false>();

            PipeCommand* command = reinterpret_cast<PipeCommand*>(lpBuffer->Buffer);
            command->Command = CommandType::AddAllocation;

            command->Data.Time = dt;
            strcpy_s(command->Data.Filename, filename);
            command->Data.LineNumber = line;
            strcpy_s(command->Data.Function, function);
            command->Data.Pointer = reinterpret_cast<ULONG_PTR>(ptr);
            command->Data.SizeOfPointerData = sizeOfPtrData;

            // capture back trace
            LPVOID aStack[ARRAYSIZE(command->Data.Stack)];
            command->Data.Frames = g_callstackCapturing.CaptureStackBackTracePtr(1, ARRAYSIZE(aStack), aStack, nullptr);

            for (USHORT i = 0; i < command->Data.Frames; ++i)
            {
                command->Data.Stack[i] = (ULONGLONG)aStack[i];
            }

            lpBuffer->Length = sizeof(PipeCommand);
            SendBuffer(lpBuffer);
            
            //ReleaseSRWLockExclusive(&g_entryLock);
        }

        NL_NOINLINE void RemoveAllocation(void* ptr)
        {
            if (g_hPipe == INVALID_HANDLE_VALUE)
                return;

            //if (!TryAcquireSRWLockExclusive(&g_entryLock))
                //return;

            auto lpBuffer = AllocateOverlappedBuffer<false>();

            PipeCommand* command = reinterpret_cast<PipeCommand*>(lpBuffer->Buffer);
            command->Command = CommandType::RemoveAllocation;

            command->Pointer = reinterpret_cast<ULONG_PTR>(ptr);

            lpBuffer->Length = sizeof(PipeCommand);
            SendBuffer(lpBuffer);

            //ReleaseSRWLockExclusive(&g_entryLock);
        }

        size_t SnapToVirtualPage(size_t size)
        {
            static DWORD dwPageSize = 0;

            if (dwPageSize == 0)
            {
                SYSTEM_INFO si;
                GetSystemInfo(&si);
                dwPageSize = si.dwPageSize;
            }

            return ((size + (dwPageSize - 1)) / dwPageSize) * dwPageSize;
        }

        void ResolvePointerData(LONG_PTR ptr, PointerInfo* pi)
        {
            SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) * 256 * sizeof(char), 1);
            ZeroMemory(symbol, sizeof(SYMBOL_INFO) + 256 * sizeof(char));
            symbol->MaxNameLen = 255;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

            if (!SymFromAddr(g_callstackCapturing.hProcess, ptr, nullptr, symbol))
            {
                pi->Pointer = (ULONGLONG)ptr;
                strcpy_s(pi->Function, "<Unresolved Pointer>");
                return;
            }

            strcpy_s(pi->Function, symbol->Name);
        }

        DWORD WINAPI AllocationTraceThread(LPVOID)
        {
            DWORD dwBytesTransferred;
            ULONG_PTR ulCompletionKey;
            OverlappedEx* overlapped;

            for (;;)
            {
                if (!GetQueuedCompletionStatus(g_hIocp, &dwBytesTransferred, &ulCompletionKey, (LPOVERLAPPED*)&overlapped, INFINITE))
                {
                    DWORD dwCode = GetLastError();
                    if (dwCode == ERROR_BROKEN_PIPE)
                    {
                        // disconnected
#ifdef __DBG_TRACING
                        nl::logger::GetGlobalLogger()->Log(
                            nl::LogType::Warn,
                            __FILE__,
                            __LINE__,
                            __FUNCTION__,
                            "Broken pipe");
#endif
                    }
                    else
                    {
#ifdef __DBG_TRACING
                        char msg[1024];
                        sprintf_s(msg, "GetQueuedCompletionStatus error: %u", dwCode);

                        nl::logger::GetGlobalLogger()->Log(
                            nl::LogType::Error,
                            __FILE__,
                            __LINE__,
                            __FUNCTION__,
                            msg);
#endif
                    }

                    continue;
                }

                if (dwBytesTransferred == 0 &&
                    ulCompletionKey == 0 &&
                    overlapped == nullptr)
                    break;

                if (overlapped->Event == IOEvent::Read)
                {
                    if (dwBytesTransferred == 0)
                    {
#ifdef __DBG_TRACING
                        nl::logger::GetGlobalLogger()->Log(
                            nl::LogType::Debug,
                            __FILE__,
                            __LINE__,
                            __FUNCTION__,
                            "Pipe destroyed");
#endif
                    }
                    else
                    {
                        char msg[1024];
                        sprintf_s(msg, "Pipe read %u b\n", dwBytesTransferred);

                        nl::logger::GetGlobalLogger()->Log(
                            nl::LogType::Error,
                            __FILE__,
                            __LINE__,
                            __FUNCTION__,
                            msg);

                        // read command from client, like request for pointer info
                        auto cmd = *(int*)overlapped->lpBuffer->Buffer;
                        if (cmd == 3)
                        {
                            // request for pointer info
                            auto request_id = *(LONGLONG*)(overlapped->lpBuffer->Buffer + sizeof(int));
                            auto pointer = *(ULONGLONG*)(overlapped->lpBuffer->Buffer + sizeof(int) + sizeof(LONGLONG));

                            auto lpBuffer = AllocateOverlappedBuffer<true>();
                            PipeCommand* pipeCommand = reinterpret_cast<PipeCommand*>(lpBuffer->Buffer);
                            pipeCommand->Command = CommandType::PointerInfo;
                            pipeCommand->PointerInfoRequest.RequestId = request_id;

                            ResolvePointerData((LONG_PTR)pointer, &pipeCommand->PointerInfoRequest.Info);

                            sprintf_s(msg, "resolve ptr %p -> %s\n", reinterpret_cast<void*>(pointer), pipeCommand->PointerInfoRequest.Info.Function);

                            nl::logger::GetGlobalLogger()->Log(
                                nl::LogType::Error,
                                __FILE__,
                                __LINE__,
                                __FUNCTION__,
                                msg);

                            lpBuffer->Length = sizeof(PipeCommand);
                            SendBuffer(lpBuffer, true);
                        }
                        else
                        {
                            auto msg_buf = AllocateOverlappedBuffer<false>();

                            char* p = msg_buf->Buffer;
                            p += sprintf(p, "Unknown bytes (%uB) => ", dwBytesTransferred);

                            for (DWORD i = 0; i < dwBytesTransferred; ++i)
                            {
                                p += sprintf(p, "%02x", (unsigned char)overlapped->lpBuffer->Buffer[i]);
                            }

                            nl::logger::GetGlobalLogger()->Log(
                                nl::LogType::Error,
                                __FILE__,
                                __LINE__,
                                __FUNCTION__,
                                msg_buf->Buffer);

                            FreeOverlappedBuffer(msg_buf);
                        }

                        if (!ReadFile(g_hPipe, overlapped->lpBuffer->Buffer, ARRAYSIZE(overlapped->lpBuffer->Buffer), nullptr, &overlapped->Overlapped))
                        {
                            DWORD dwCode = GetLastError();
                            if (dwCode != ERROR_IO_PENDING)
                            {
#ifdef __DBG_TRACING
                                char msg[1024];
                                sprintf_s(msg, "err ReadFile code: %u", dwCode);

                                nl::logger::GetGlobalLogger()->Log(
                                    nl::LogType::Error,
                                    __FILE__,
                                    __LINE__,
                                    __FUNCTION__,
                                    msg);
#endif
                            }
                        }
                    }
                }
                else if (overlapped->Event == IOEvent::Write)
                {
                    overlapped->lpBuffer->Offset += dwBytesTransferred;
                    overlapped->lpBuffer->Length -= dwBytesTransferred;
                    if (overlapped->lpBuffer->Length != 0)
                    {
                        // need to send more..
#ifdef __DBG_TRACING
                        char msg[1024];

                        sprintf_s(msg, "INCOMPLETE - %u remaining\n", (DWORD)overlapped->lpBuffer->Length);
                        nl::logger::GetGlobalLogger()->Log(
                            nl::LogType::Warn,
                            __FILE__,
                            __LINE__,
                            __FUNCTION__,
                            msg);
#endif

                        if (!WriteFile(g_hPipe, overlapped->lpBuffer->Buffer + overlapped->lpBuffer->Offset, (DWORD)overlapped->lpBuffer->Length, nullptr, &overlapped->Overlapped))
                        {
                            DWORD dwCode = GetLastError();
                            if (dwCode != ERROR_IO_PENDING)
                            {
#ifdef __DBG_TRACING
                                char msg[1024];
                                sprintf_s(msg, "err WriteFile code: %u", dwCode);

                                nl::logger::GetGlobalLogger()->Log(
                                    nl::LogType::Error,
                                    __FILE__,
                                    __LINE__,
                                    __FUNCTION__,
                                    msg);
#endif
                            }
                        }
                    }
                    else
                    {
                        FreeOverlappedBuffer(overlapped->lpBuffer);

                        // more data to send?
                        AcquireSRWLockExclusive(&g_sendBufferLock);
                        overlapped->lpBuffer = g_sendBuffer.PopHead();
                        if (overlapped->lpBuffer == nullptr)
                            g_bIsSending = false;

                        if (overlapped->lpBuffer)
                        {
                            PipeCommand* cmd = reinterpret_cast<PipeCommand*>(overlapped->lpBuffer);
                            cmd->PacketIndex = nl::threading::Interlocked::Increment(&g_lNextPacketIndex);

                            // write to pipe first, then release lock
                            if (!WriteFile(g_hPipe, overlapped->lpBuffer->Buffer + overlapped->lpBuffer->Offset, (DWORD)overlapped->lpBuffer->Length, nullptr, &overlapped->Overlapped))
                            {
                                DWORD dwCode = GetLastError();
                                if (dwCode != ERROR_IO_PENDING)
                                {
#ifdef __DBG_TRACING
                                    char msg[1024];
                                    sprintf_s(msg, "err WriteFile code: %u", dwCode);

                                    nl::logger::GetGlobalLogger()->Log(
                                        nl::LogType::Error,
                                        __FILE__,
                                        __LINE__,
                                        __FUNCTION__,
                                        msg);
#endif
                                }
                            }

                            ReleaseSRWLockExclusive(&g_sendBufferLock);
                        }
                        else
                        {
                            // release lock first, then free overlapped
                            ReleaseSRWLockExclusive(&g_sendBufferLock);
                            FreeOverlapped(overlapped);
                        }
                    }
                }
                else
                    throw Exception("Unknown trace event");
            }

            return 0;
        }
    }
}

#endif