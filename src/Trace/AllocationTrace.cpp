#include "StdAfx.h"

#include <NativeLib/Trace/AllocationTrace.h>

#include <NativeLib/Platform/DbgHelpWin.h>

namespace nl
{
    namespace trace
    {
        DWORD WINAPI AllocationTraceThread(LPVOID);

        typedef USHORT(WINAPI* CaptureStackBackTraceType)(ULONG FramesToSkip, ULONG FramesToCapture, PVOID* BackTrace, PULONG BackTraceHash);

        SRWLOCK g_lock = SRWLOCK_INIT;
        char g_tracingFilename[MAX_PATH] = {};
        HANDLE g_hFile = INVALID_HANDLE_VALUE;
        HANDLE g_hThread = nullptr;
        HANDLE g_hIocp = nullptr;
        LARGE_INTEGER g_liFrequency;
        LARGE_INTEGER g_liStart;
        
        struct
        {
            HANDLE hProcess = nullptr;
            HMODULE hKernel = nullptr;
            CaptureStackBackTraceType CaptureStackBackTracePtr = nullptr;

        } g_callstackCapturing;

#pragma pack(push, 1)
        struct FunctionData
        {
            //char Filename[188];
            //int LineNumber;
            char pad[192];
            char Function[64];
        };
#pragma pack(pop)

        static_assert(sizeof(FunctionData) == 256, "The FunctionData is expected to be exactly 256 bytes");

        struct OverlappedEx
        {
            OVERLAPPED Overlapped;
            
            double Time;
            FunctionData Data;
            void* Ptr;
            LPVOID Stack[32];
            USHORT Frames;
        };

        static OverlappedEx* g_overlappedPtr = new OverlappedEx;

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

        void Setup(const char* filename)
        {
            nl_assert_if_debug(g_hFile == INVALID_HANDLE_VALUE);

            InitializeCallstackCapturing();

            g_hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (g_hFile == INVALID_HANDLE_VALUE)
                throw Win32Exception();

            QueryPerformanceFrequency(&g_liFrequency);
            QueryPerformanceCounter(&g_liStart);

            g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);

            g_hThread = CreateThread(nullptr, 0, AllocationTraceThread, nullptr, 0, nullptr);
        }

        void Terminate()
        {
            nl_assert_if_debug(g_hFile != INVALID_HANDLE_VALUE);

            PostQueuedCompletionStatus(g_hIocp, 0, 0, nullptr);
            WaitForSingleObject(g_hThread, INFINITE);
            
            CloseHandle(g_hThread);
            g_hThread = nullptr;
            
            CloseHandle(g_hIocp);
            g_hIocp = nullptr;

            CloseHandle(g_hFile);
            g_hFile = INVALID_HANDLE_VALUE;
        }

        __declspec(noinline) void AddAllocation(const char* filename, int line, const char* function, void* ptr)
        {
            LARGE_INTEGER li;
            QueryPerformanceCounter(&li);
            double dt = double(li.QuadPart - g_liStart.QuadPart) / g_liFrequency.QuadPart;

            OverlappedEx* overlapped = nullptr;
            for (;;)
            {
                overlapped = (OverlappedEx*)InterlockedExchangePointer((volatile PVOID*)&g_overlappedPtr, nullptr);
                if (!overlapped)
                {
                    Sleep(0);
                    continue;
                }
            }

            overlapped->Time = dt;
            strcpy_s(overlapped->Filename, filename);
            overlapped->LineNumber = line;
            strcpy_s(overlapped->Function, function);
            overlapped->Ptr = ptr;

            overlapped->Frames = g_callstackCapturing.CaptureStackBackTracePtr(1, ARRAYSIZE(overlapped->Stack), overlapped->Stack, nullptr);

            PostQueuedCompletionStatus(g_hIocp, 0, 1, &overlapped->Overlapped);
        }

        __declspec(noinline) void RemoveAllocation(void* ptr)
        {
        }

        DWORD WINAPI AllocationTraceThread(LPVOID)
        {
            size_t sizeFunctionMapping = 1024 * 1024 * 64; // 64mb for 262144 pointer->function mapping
            size_t sizeFreeList = 0;

            // map of a function ptr to a filename, line and function name

            LARGE_INTEGER li;
            li.QuadPart = (LONGLONG)fileSize;
            SetFilePointerEx(g_hFile, li, nullptr, FILE_BEGIN);
            SetEndOfFile(g_hFile);

            HANDLE hFileMapping = CreateFileMapping(g_hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
            char* dataPtr = (char*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            DWORD dwBytesTransferred;
            ULONG_PTR ulCompletionKey;
            OverlappedEx* overlapped;

            for (;;)
            {
                GetQueuedCompletionStatus(g_hIocp, &dwBytesTransferred, &ulCompletionKey, (LPOVERLAPPED*)&overlapped, INFINITE);

                if (dwBytesTransferred == 0 &&
                    ulCompletionKey == 0 &&
                    overlapped == nullptr)
                    break;

                if (ulCompletionKey == 1)
                {
                    // ... process thingy

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

                    // put back in usable ptr variable
                    InterlockedExchangePointer((volatile PVOID*)&g_overlappedPtr, overlapped);
                }
            }

            UnmapViewOfFile(dataPtr);
            CloseHandle(hFileMapping);
            return 0;
        }
    }
}