#include "StdAfx.h"

#include <NativeLib/Trace/AllocationTrace.h>

#include <NativeLib/Platform/DbgHelpWin.h>

#define __DBG_TRACING

namespace nl
{
    namespace trace
    {
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

        LONGLONG g_lNextPacketIndex = 0;

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

        struct OverlappedEx
        {
            OVERLAPPED Overlapped;
            IOEvent Event;
            char Buffer[4096];
        };

        SRWLOCK g_overlappedPoolLock = SRWLOCK_INIT;
        OverlappedEx* g_overlappedPointers[1024];
        size_t g_overlappedPointersCount = 0;

        OverlappedEx* AllocateOverlapped(IOEvent event)
        {
            OverlappedEx* overlapped = nullptr;
            for (;;)
            {
                AcquireSRWLockExclusive(&g_overlappedPoolLock);

                if (g_overlappedPointersCount != 0)
                    overlapped = g_overlappedPointers[--g_overlappedPointersCount];

                ReleaseSRWLockExclusive(&g_overlappedPoolLock);

                if (overlapped)
                    break;

                Sleep(0);
            }

            ZeroMemory(overlapped, sizeof(OverlappedEx));

            overlapped->Event = event;
            return overlapped;
        }

        void FreeOverlapped(OverlappedEx* overlapped)
        {
            AcquireSRWLockExclusive(&g_overlappedPoolLock);
            g_overlappedPointers[g_overlappedPointersCount++] = overlapped;
            ReleaseSRWLockExclusive(&g_overlappedPoolLock);
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

            while (g_overlappedPointersCount < ARRAYSIZE(g_overlappedPointers))
            {
                g_overlappedPointers[g_overlappedPointersCount++] = new OverlappedEx;
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
            ReadFile(g_hPipe, overlappedRead->Buffer, ARRAYSIZE(overlappedRead->Buffer), nullptr, &overlappedRead->Overlapped);
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

            while (g_overlappedPointersCount != 0)
            {
                delete g_overlappedPointers[--g_overlappedPointersCount];
            }
        }

        __declspec(noinline) void AddAllocation(const char* filename, int line, const char* function, void* ptr, size_t sizeOfPtrData)
        {
            if (g_hPipe == INVALID_HANDLE_VALUE)
                return;

            LARGE_INTEGER li;
            QueryPerformanceCounter(&li);
            double dt = double(li.QuadPart - g_liStart.QuadPart) / g_liFrequency.QuadPart;

            OverlappedEx* overlapped = AllocateOverlapped(IOEvent::Write);

            auto packet_index = InterlockedIncrement64(&g_lNextPacketIndex);

            PipeCommand* command = reinterpret_cast<PipeCommand*>(overlapped->Buffer);
            command->PacketIndex = packet_index;
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

            WriteFile(g_hPipe, overlapped->Buffer, sizeof(PipeCommand), nullptr, &overlapped->Overlapped);
        }

        __declspec(noinline) void RemoveAllocation(void* ptr)
        {
            if (g_hPipe == INVALID_HANDLE_VALUE)
                return;

            OverlappedEx* overlapped = AllocateOverlapped(IOEvent::Write);

            PipeCommand* command = reinterpret_cast<PipeCommand*>(overlapped->Buffer);
            command->Command = CommandType::RemoveAllocation;

            command->Pointer = reinterpret_cast<ULONG_PTR>(ptr);

            WriteFile(g_hPipe, overlapped->Buffer, sizeof(PipeCommand), nullptr, &overlapped->Overlapped);
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
                        printf("broken pipe\n");
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
#ifdef __DBG_TRACING
                    printf(__FUNCTION__ " - read %u bytes\n", dwBytesTransferred);
#endif

                    if (dwBytesTransferred == 0)
                    {
#ifdef __DBG_TRACING
                        printf("pipe destroyed\n");
#endif
                    }
                    else
                    {
                        // read command from client, like request for pointer info
                        auto cmd = *(int*)overlapped->Buffer;
                        if (cmd == 3)
                        {
                            // request for pointer info
                            auto request_id = *(LONGLONG*)(overlapped->Buffer + sizeof(int));
                            auto pointer = *(ULONGLONG*)(overlapped->Buffer + sizeof(int) + sizeof(LONGLONG));

                            auto packet_index = InterlockedIncrement64(&g_lNextPacketIndex);

                            PipeCommand pipeCommand = {};
                            pipeCommand.PacketIndex = packet_index;
                            pipeCommand.Command = CommandType::PointerInfo;
                            pipeCommand.PointerInfoRequest.RequestId = request_id;

                            ResolvePointerData((LONG_PTR)pointer, &pipeCommand.PointerInfoRequest.Info);

                            //printf("resolve ptr %p -> %s\n", reinterpret_cast<void*>(pointer), pipeCommand.PointerInfoRequest.Info.Function);

                            auto overlappedSend = AllocateOverlapped(IOEvent::Write);
                            memcpy(overlappedSend->Buffer, &pipeCommand, sizeof(PipeCommand));
                            WriteFile(g_hPipe, overlappedSend->Buffer, sizeof(PipeCommand), nullptr, &overlappedSend->Overlapped);
                        }

                        ReadFile(g_hPipe, overlapped->Buffer, ARRAYSIZE(overlapped->Buffer), nullptr, &overlapped->Overlapped);
                    }
                }
                else if (overlapped->Event == IOEvent::Write)
                {
#ifdef __DBG_TRACING
                    printf(__FUNCTION__ " - wrote %u bytes\n", dwBytesTransferred);
#endif

                    FreeOverlapped(overlapped);
                }
                else
                    throw Exception("Unknown trace event");
            }

            return 0;
        }
    }
}