#pragma once

#include <NativeLib/Allocators.h>
#include <NativeLib/IO/Stream.h>

namespace nl
{
    namespace io
    {
        class MemoryStream : public Stream
        {
        public:
            MemoryStream(int64_t initial_capacity = 1024);

            virtual bool CanSeek() const;
            virtual bool CanRead() const;
            virtual bool CanWrite() const;

            virtual int64_t GetPosition() const;
            virtual int64_t GetLength() const;
            virtual int64_t Seek(int64_t offset, SeekMode mode = SeekMode::Begin);
            virtual void SetLength(int64_t length);

            virtual int64_t Read(void* lp, int64_t numberOfBytesToRead);
            virtual int64_t Write(const void* lp, int64_t numberOfBytesToWrite);

            nl::memory::Memory& GetMemory()
            {
                return m_memory;
            }

        private:
            void AdjustSize(int64_t new_minimum_size);

            nl::memory::Memory m_memory;
            int64_t m_position;
            int64_t m_length;
        };
    }
}