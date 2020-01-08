#pragma once

#include <NativeLib/IO/Stream.h>

#include <NativeLib/Platform/Windows.h>

namespace nl
{
    namespace io
    {
        enum class CreateMode
        {
            CreateNew,
            CreateAlways,
            OpenExisting,
            OpenAlways,
            TruncateExisting,
        };

        enum class SeekMode
        {
            Begin,
            Current,
            End
        };

        class File
        {
        public:
            File();
            ~File();

            File(const File&) = delete;
            File(File&&) noexcept = delete;
            File& operator =(const File&) = delete;
            File& operator =(File&&) noexcept = delete;

            operator bool() const;

            bool IsOpen() const;
            long long GetPosition() const;
            long long GetSize() const;

            void Close();
            void Seek(SeekMode mode, long long offset);
            size_t Read(void* lp, size_t count);
            size_t Write(const void* lp, size_t count);
            
            static File Open(const char* filename, CreateMode mode, bool writable = true);

        protected:
            File(HANDLE hFile);

        private:
            HANDLE m_hFile;
        };
    }
}