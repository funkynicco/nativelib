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
        class FileStream : public Stream
        {
        public:
            FileStream();
            ~FileStream();

            FileStream(const FileStream&) = delete;
            FileStream(FileStream&&) noexcept = delete;
            FileStream& operator =(const FileStream&) = delete;
            FileStream& operator =(FileStream&&) noexcept = delete;

            operator bool() const;

            virtual bool CanSeek() const override;
            virtual bool CanRead() const override;
            virtual bool CanWrite() const override;

            bool IsOpen() const;
            virtual int64_t GetPosition() const override;
            virtual int64_t GetLength() const override;
            virtual void SetLength(int64_t length) override;
            virtual void Flush() override;
            virtual void Close() override;

            virtual int64_t Seek(int64_t offset, SeekMode mode) override;
            virtual int64_t Read(void* lp, int64_t numberOfBytesToRead) override;
            virtual int64_t Write(const void* lp, int64_t numberOfBytesToWrite) override;
            
            static FileStream Open(std::string_view filename, CreateMode mode, bool writable = true);

        protected:
            FileStream(systemlayer::FileHandle fp);

        private:
            systemlayer::FileHandle m_fp;
        };
    }
}