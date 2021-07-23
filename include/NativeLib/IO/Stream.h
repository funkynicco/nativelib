#pragma once

#include <NativeLib/IO/IOEnum.h>

namespace nl
{
    namespace io
    {
        class Stream
        {
        public:
            virtual bool CanSeek() const = 0;
            virtual bool CanRead() const = 0;
            virtual bool CanWrite() const = 0;

            virtual int64_t GetPosition() const;
            virtual int64_t GetLength() const;
            virtual int64_t Seek(int64_t offset, SeekMode mode = SeekMode::Begin);
            virtual void SetLength(int64_t length);
            virtual void Flush();
            virtual void Close();

            virtual int64_t Read(void* lp, int64_t numberOfBytesToRead) = 0;
            virtual int64_t Write(const void* lp, int64_t numberOfBytesToWrite) = 0;

            // Copies the remaining data of this stream to the target.
            virtual void CopyTo(Stream* stream);
        };
    }
}