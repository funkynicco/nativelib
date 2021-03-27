#include "StdAfx.h"

#ifdef NL_PLATFORM_WINDOWS

#include <NativeLib/SystemLayer/SystemLayer.h>

#include "SystemLayerWindows.h"

#include <NativeLib/Exceptions.h>

#include <Windows.h>

namespace nl::systemlayer::defaults
{
    static FileHandle Open(const char* filename, nl::io::CreateMode mode, bool writable)
    {
        DWORD dwCreationMode = 0;
        switch (mode)
        {
        case nl::io::CreateMode::CreateNew:
            dwCreationMode = CREATE_NEW;
            break;
        case nl::io::CreateMode::CreateAlways:
            dwCreationMode = CREATE_ALWAYS;
            break;
        case nl::io::CreateMode::OpenExisting:
            dwCreationMode = OPEN_EXISTING;
            break;
        case nl::io::CreateMode::OpenAlways:
            dwCreationMode = OPEN_ALWAYS;
            break;
        case nl::io::CreateMode::TruncateExisting:
            dwCreationMode = TRUNCATE_EXISTING;
            break;
        }

        DWORD dwDesiredAccess = GENERIC_READ;
        if (writable)
            dwDesiredAccess |= GENERIC_WRITE;

        HANDLE hFile = ::CreateFileA(filename, dwDesiredAccess, FILE_SHARE_READ, nullptr, dwCreationMode, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return 0;

        return (FileHandle)hFile;
    }

    static void Close(FileHandle fp)
    {
        HANDLE hFile = (HANDLE)fp;
        ::CloseHandle(hFile);
    }

    static int64_t GetPosition(FileHandle fp)
    {
        HANDLE hFile = (HANDLE)fp;

        LARGE_INTEGER li;
        if (!SetFilePointerEx(hFile, {}, &li, FILE_CURRENT))
            throw IOException(IOException::SeekFailed);

        return (int64_t)li.QuadPart;
    }

    static int64_t GetSize(FileHandle fp)
    {
        HANDLE hFile = (HANDLE)fp;

        LARGE_INTEGER li;
        if (!GetFileSizeEx(hFile, &li))
            throw Exception("Failed to get size");

        return (int64_t)li.QuadPart;
    }

    static bool Seek(FileHandle fp, int64_t offset, nl::io::SeekMode mode)
    {
        HANDLE hFile = (HANDLE)fp;

        DWORD dwMoveMethod = FILE_BEGIN;
        switch (mode)
        {
        case nl::io::SeekMode::Begin: dwMoveMethod = FILE_BEGIN; break;
        case nl::io::SeekMode::Current: dwMoveMethod = FILE_CURRENT; break;
        case nl::io::SeekMode::End: dwMoveMethod = FILE_END; break;
        }

        LARGE_INTEGER li;
        li.QuadPart = offset;
        if (!SetFilePointerEx(hFile, li, nullptr, dwMoveMethod))
            return false;

        return true;
    }

    static size_t Read(FileHandle fp, void* ptr, size_t count)
    {
        HANDLE hFile = (HANDLE)fp;
        
        size_t remaining = count;
        while (remaining != 0)
        {
            DWORD dw;
            if (!ReadFile(hFile, ptr, (DWORD)remaining, &dw, nullptr) ||
                dw == 0)
                break;

            ptr = (uint8_t*)ptr + dw;
            remaining -= dw;
        }

        return count - remaining;
    }

    static size_t Write(FileHandle fp, const void* ptr, size_t count)
    {
        HANDLE hFile = (HANDLE)fp;
        
        size_t remaining = count;
        while (remaining != 0)
        {
            DWORD dw;
            if (!WriteFile(hFile, ptr, (DWORD)remaining, &dw, nullptr) ||
                dw == 0)
                break;

            ptr = (const uint8_t*)ptr + dw;
            remaining -= dw;
        }

        return count - remaining;
    }

    static bool Flush(FileHandle fp)
    {
        HANDLE hFile = (HANDLE)fp;
        if (!::FlushFileBuffers(hFile))
            return false;

        return true;
    }

    static bool SetEndOfFile(FileHandle fp)
    {
        HANDLE hFile = (HANDLE)fp;
        if (!::SetEndOfFile(hFile))
            return false;

        return true;
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