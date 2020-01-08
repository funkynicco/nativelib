#include "StdAfx.h"

#include <NativeLib/IO/File.h>

namespace nl
{
    namespace io
    {
        File::File() :
            m_hFile(INVALID_HANDLE_VALUE)
        {
        }

        File::File(HANDLE hFile) :
            m_hFile(hFile)
        {
        }

        File::~File()
        {
            Close();
        }

        File::operator bool() const
        {
            return m_hFile != INVALID_HANDLE_VALUE;
        }

        bool File::IsOpen() const
        {
            return m_hFile != INVALID_HANDLE_VALUE;
        }

        long long File::GetPosition() const
        {
            LARGE_INTEGER li = {};
            SetFilePointerEx(m_hFile, {}, &li, FILE_CURRENT);
            return li.QuadPart;
        }

        long long File::GetSize() const
        {
            LARGE_INTEGER li = {};
            GetFileSizeEx(m_hFile, &li);
            return li.QuadPart;
        }

        void File::Close()
        {
            if (m_hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
            }
        }

        void File::Seek(SeekMode mode, long long offset)
        {
            LARGE_INTEGER li;
            li.QuadPart = offset;

            DWORD dwMoveMethod = FILE_BEGIN;
            switch (mode)
            {
            case SeekMode::Begin:
                dwMoveMethod = FILE_BEGIN;
                break;
            case SeekMode::Current:
                dwMoveMethod = FILE_CURRENT;
                break;
            case SeekMode::End:
                dwMoveMethod = FILE_END;
                break;
            }

            SetFilePointerEx(m_hFile, li, nullptr, dwMoveMethod);
        }

        size_t File::Read(void* lp, size_t count)
        {
            size_t remaining = count;
            while (remaining != 0)
            {
                DWORD dw;
                if (!ReadFile(m_hFile, lp, remaining, &dw, nullptr) ||
                    dw == 0)
                    break;

                lp = (unsigned char*)lp + dw;
                remaining -= dw;
            }

            return count - remaining;
        }

        size_t File::Write(const void* lp, size_t count)
        {
            size_t remaining = count;
            while (remaining != 0)
            {
                DWORD dw;
                if (!WriteFile(m_hFile, lp, remaining, &dw, nullptr) ||
                    dw == 0)
                    break;

                lp = (const unsigned char*)lp + dw;
                remaining -= dw;
            }

            return count - remaining;
        }

        File File::Open(const char* filename, CreateMode mode, bool writable)
        {
            DWORD dwDesiredAccess = GENERIC_READ;
            if (writable)
                dwDesiredAccess |= GENERIC_WRITE;

            DWORD dwCreationDisposition = 0;
            switch (mode)
            {
            case CreateMode::CreateNew:
                dwCreationDisposition = CREATE_NEW;
                break;
            case CreateMode::CreateAlways:
                dwCreationDisposition = CREATE_ALWAYS;
                break;
            case CreateMode::OpenExisting:
                dwCreationDisposition = OPEN_EXISTING;
                break;
            case CreateMode::OpenAlways:
                dwCreationDisposition = OPEN_ALWAYS;
                break;
            case CreateMode::TruncateExisting:
                dwCreationDisposition = TRUNCATE_EXISTING;
                break;
            }

            return File(CreateFileA(filename, dwDesiredAccess, FILE_SHARE_READ, nullptr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr));
        }
    }
}