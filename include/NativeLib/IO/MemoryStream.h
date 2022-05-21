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

            virtual bool CanSeek() const override;
            virtual bool CanRead() const override;
            virtual bool CanWrite() const override;

            virtual int64_t GetPosition() const override;
            virtual int64_t GetLength() const override;
            virtual int64_t Seek(int64_t offset, SeekMode mode = SeekMode::Begin) override;
            virtual void SetLength(int64_t length) override;

            virtual int64_t Read(void* lp, int64_t numberOfBytesToRead) override;
            virtual int64_t Write(const void* lp, int64_t numberOfBytesToWrite) override;

            virtual void Remove(int64_t offset, int64_t length);
            virtual void Insert(int64_t offset, int64_t length);

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