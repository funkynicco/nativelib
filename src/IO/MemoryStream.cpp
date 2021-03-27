#include "StdAfx.h"

#include <NativeLib/IO/MemoryStream.h>
#include <NativeLib/Util.h>

namespace nl
{
    namespace io
    {
        MemoryStream::MemoryStream(int64_t initial_capacity) :
            m_memory(nl::memory::Memory::Allocate(initial_capacity)),
            m_position(0),
            m_length(0)
        {
        }

        bool MemoryStream::CanSeek() const
        {
            return true;
        }

        bool MemoryStream::CanRead() const
        {
            return true;
        }

        bool MemoryStream::CanWrite() const
        {
            return true;
        }

        int64_t MemoryStream::GetPosition() const
        {
            return m_position;
        }

        int64_t MemoryStream::GetLength() const
        {
            return m_length;
        }

        int64_t MemoryStream::Seek(int64_t offset, SeekMode mode)
        {
            int64_t pos = 0;

            switch (mode)
            {
            case SeekMode::Begin:
                pos = offset;
                break;
            case SeekMode::Current:
                pos = m_position + offset;
                break;
            case SeekMode::End:
                pos = m_length - offset;
                break;
            }

            if (pos < 0)
                throw ArgumentException("The resulting position cannot be negative.");

            return m_position;
        }

        void MemoryStream::SetLength(int64_t length)
        {
            if (length > m_length)
                AdjustSize(length);

            m_length = length;
            if (m_position > m_length)
                m_position = m_length;
        }

        int64_t MemoryStream::Read(void* lp, int64_t numberOfBytesToRead)
        {
            numberOfBytesToRead = nl::util::Min(m_length - m_position, numberOfBytesToRead);
            memcpy(lp, (const char*)m_memory.Get() + m_position, numberOfBytesToRead);
            m_position += numberOfBytesToRead;
            return numberOfBytesToRead;
        }

        int64_t MemoryStream::Write(const void* lp, int64_t numberOfBytesToWrite)
        {
            AdjustSize(m_position + numberOfBytesToWrite);

            memcpy(m_memory.Get<char>() + m_position, lp, numberOfBytesToWrite);
            m_position += numberOfBytesToWrite;
            if (m_length < m_position)
                m_length = m_position;

            return numberOfBytesToWrite;
        }

        void MemoryStream::AdjustSize(int64_t new_minimum_size /* ,bool shrink */)
        {
            if ((int64_t)m_memory.GetSize() > new_minimum_size)
                return;

            int64_t new_size = m_memory.GetSize();
            while (new_size < new_minimum_size)
            {
                if (new_size < 1048576) // 1 MB
                    new_size *= 2;
                else
                    new_size += 1048576; // add 1 MB maximum each loop
            }

            auto new_memory = nl::memory::Memory::Allocate(new_size);
            memcpy(new_memory, m_memory, m_length);
            m_memory = std::move(new_memory);
        }
    }
}