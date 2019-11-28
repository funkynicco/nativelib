#include "StdAfx.h"

#include <NativeLib/Trace/AllocationTrace.h>

#include <NativeLib/Platform/DbgHelpWin.h>

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
            RemoveAllocation
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

            LPVOID Stack[32];
            StackInfo StackInfo[32];
        };

        struct PipeCommand
        {
            CommandType Command;

            union
            {
                FunctionData Data;
                void* Pointer;
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

        __declspec(noinline) void AddAllocation(const char* filename, int line, const char* function, void* ptr)
        {
            LARGE_INTEGER li;
            QueryPerformanceCounter(&li);
            double dt = double(li.QuadPart - g_liStart.QuadPart) / g_liFrequency.QuadPart;

            OverlappedEx* overlapped = AllocateOverlapped(IOEvent::Write);

            PipeCommand* command = reinterpret_cast<PipeCommand*>(overlapped->Buffer);
            command->Command = CommandType::AddAllocation;

            command->Data.Time = dt;
            strcpy_s(command->Data.Filename, filename);
            command->Data.LineNumber = line;
            strcpy_s(command->Data.Function, function);
            command->Data.Pointer = reinterpret_cast<ULONG_PTR>(ptr);

            LPVOID stack[32];
            auto frames = g_callstackCapturing.CaptureStackBackTracePtr(1, ARRAYSIZE(stack), stack, nullptr);



            WriteFile(g_hPipe, overlapped->Buffer, sizeof(PipeCommand), nullptr, &overlapped->Overlapped);
        }

        __declspec(noinline) void RemoveAllocation(void* ptr)
        {
            OverlappedEx* overlapped = AllocateOverlapped(IOEvent::Write);

            PipeCommand* command = reinterpret_cast<PipeCommand*>(overlapped->Buffer);
            command->Command = CommandType::RemoveAllocation;

            command->Pointer = ptr;

            WriteFile(g_hPipe, overlapped->Buffer, sizeof(PipeCommand), nullptr, &overlapped->Overlapped);
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
                        printf("broken pipe\n");
                    }
                    else
                    {
                        //char msg[1024];
                        //sprintf_s(msg, "unknown code %u", dwCode);
                        //MessageBoxA(nullptr, msg, "", MB_OK);
                    }

                    continue;
                }

                if (dwBytesTransferred == 0 &&
                    ulCompletionKey == 0 &&
                    overlapped == nullptr)
                    break;

                if (overlapped->Event == IOEvent::Read)
                {
                    printf(__FUNCTION__ " - read %u bytes\n", dwBytesTransferred);

                    if (dwBytesTransferred == 0)
                    {
                        printf("pipe destroyed\n");
                    }
                    else
                        ReadFile(g_hPipe, overlapped->Buffer, ARRAYSIZE(overlapped->Buffer), nullptr, &overlapped->Overlapped);
                }
                else if (overlapped->Event == IOEvent::Write)
                {
                    printf(__FUNCTION__ " - wrote %u bytes\n", dwBytesTransferred);
                    FreeOverlapped(overlapped);
                }
                else
                    throw Exception("Unknown trace event");

#if 0
                SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) * 256 * sizeof(char), 1);
                ZeroMemory(symbol, sizeof(SYMBOL_INFO) + 256 * sizeof(char));
                symbol->MaxNameLen = 255;
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

                nl::BasicString<1024> str;

                for (USHORT i = 0; i < frames; ++i)
                {
                    SymFromAddr(g_callstackCapturing.hProcess, reinterpret_cast<DWORD64>(stack[i]), nullptr, symbol);

                    if (str.GetLength() != 0)
                        str.Append("\n");

                    str.Append(nl::BasicString<128>::Format("- [{}] {}", (const void*)symbol->Address, symbol->Name));
                }
#endif
                }

            return 0;
            }
        }
    }