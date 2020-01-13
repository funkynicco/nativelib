#pragma once

#include <NativeLib/IO/Stream.h>
#include <NativeLib/SystemLayer/SystemLayer.h>

#include <NativeLib/Platform/Platform.h>
#include <NativeLib/IO/IOEnum.h>

#include <stdint.h>
#include <string_view>

namespace nl
{
    namespace io
    {
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
            int64_t GetPosition() const;
            int64_t GetSize() const;

            void Close();
            bool Seek(SeekMode mode, int64_t offset);
            size_t Read(void* lp, size_t count);
            size_t Write(const void* lp, size_t count);
            
            static File Open(std::string_view filename, CreateMode mode, bool writable = true);

        protected:
            File(systemlayer::FileHandle fp);

        private:
            systemlayer::FileHandle m_fp;
        };
    }
}