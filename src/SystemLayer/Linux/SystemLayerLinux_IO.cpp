#include "StdAfx.h"

#ifdef NL_PLATFORM_LINUX

#include <NativeLib/SystemLayer/SystemLayer.h>

//!ALLOW_INCLUDE "SystemLayerLinux.h"
#include "SystemLayerLinux.h"

#include <NativeLib/Exceptions.h>

namespace nl::systemlayer::defaults
{
    static FileHandle Open(const char* filename, nl::io::CreateMode mode, bool writable)
    {
        throw NotImplementedException();
    }

    static void Close(FileHandle fp)
    {
        throw NotImplementedException();
    }

    static int64_t GetPosition(FileHandle fp)
    {
        throw NotImplementedException();
    }

    static int64_t GetSize(FileHandle fp)
    {
        throw NotImplementedException();
    }

    static bool Seek(FileHandle fp, int64_t offset, nl::io::SeekMode mode)
    {
        throw NotImplementedException();
    }

    static int64_t Read(FileHandle fp, void* ptr, int64_t count)
    {
        throw NotImplementedException();
    }

    static int64_t Write(FileHandle fp, const void* ptr, int64_t count)
    {
        throw NotImplementedException();
    }

    static bool Flush(FileHandle fp)
    {
        throw NotImplementedException();
    }

    static bool SetEndOfFile(FileHandle fp)
    {
        throw NotImplementedException();
    }

    bool SetFileIO(SystemLayerFunctions* functions)
    {
        functions->FileOpen = Open;
        functions->FileClose = Close;
        functions->FileGetPosition = GetPosition;
        functions->FileGetSize = GetSize;
        functions->FileSeek = Seek;
        functions->FileRead = Read;
        functions->FileWrite = Write;
        functions->FileFlush = Flush;
        functions->FileSetEndOfFile = SetEndOfFile;
        return true;
    }
}

#endif