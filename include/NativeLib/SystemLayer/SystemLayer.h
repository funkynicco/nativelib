#pragma once

#include <NativeLib/Platform/Platform.h>

#include <NativeLib/IO/IOEnum.h>
#include <NativeLib/Assert.h>

#include <stdint.h>

namespace nl::systemlayer
{
    typedef uint64_t FileHandle;

    namespace delegates
    {
        // heap memory (tiny chunks of allocations)
        typedef void* TAllocateHeapMemory(size_t size);
        typedef void* TReallocateHeapMemory(void* ptr, size_t new_size);
        typedef void TFreeHeapMemory(void* ptr);

        // virtual memory (typically pages of memory)
        typedef int64_t TGetVirtualMemoryPageSize();
        typedef void* TAllocateVirtualMemory(size_t size);
        typedef void TFreeVirtualMemory(void* ptr);

        // file api
        typedef FileHandle TFileOpen(const char* filename, nl::io::CreateMode mode, bool writable);
        typedef void TFileClose(FileHandle fp);
        typedef int64_t TFileGetPosition(FileHandle fp); // get pos
        typedef int64_t TFileGetSize(FileHandle fp); // get size
        typedef bool TFileSeek(FileHandle fp, int64_t offset, nl::io::SeekMode mode); // seek
        typedef size_t TFileRead(FileHandle fp, void* ptr, size_t count); // read
        typedef size_t TFileWrite(FileHandle fp, const void* ptr, size_t count); // write
        typedef bool TFileFlush(FileHandle fp); // flush
        typedef bool TFileSetEndOfFile(FileHandle fp); // set end of file
    }

    struct SystemLayerFunctions
    {
        nl::assert::pfnAssertHandler AssertHandler;

        delegates::TAllocateHeapMemory* AllocateHeapMemory;
        delegates::TReallocateHeapMemory* ReallocateHeapMemory;
        delegates::TFreeHeapMemory* FreeHeapMemory;

        delegates::TGetVirtualMemoryPageSize* GetVirtualMemoryPageSize;
        delegates::TAllocateVirtualMemory* AllocateVirtualMemory;
        delegates::TFreeVirtualMemory* FreeVirtualMemory;

        delegates::TFileOpen* FileOpen;
        delegates::TFileClose* FileClose;
        delegates::TFileGetPosition* FileGetPosition;
        delegates::TFileGetSize* FileGetSize;
        delegates::TFileSeek* FileSeek;
        delegates::TFileRead* FileRead;
        delegates::TFileWrite* FileWrite;
        delegates::TFileFlush* FileFlush;
        delegates::TFileSetEndOfFile* FileSetEndOfFile;
    };

    const SystemLayerFunctions* GetSystemLayerFunctions();
    void SetSystemLayerFunctions(const SystemLayerFunctions* functions);
    bool GetDefaultSystemLayerFunctions(SystemLayerFunctions* functions);
    bool SetDefaultSystemLayerFunctions();
}