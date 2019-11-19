#include "StdAfx.h"
#include "DataBuffer.h"

namespace nl
{
    namespace rpc
    {
        DataBuffer::DataBuffer()
        {
            m_offset = 0;
            m_length = 0;
            m_capacity = 0;
            m_buffer = nullptr;
        }

        DataBuffer::~DataBuffer()
        {
            if (m_buffer)
                free(m_buffer);
        }

        DataBuffer::DataBuffer(DataBuffer&& buffer) noexcept
        {
            m_offset = buffer.m_offset;
            m_length = buffer.m_length;
            m_capacity = buffer.m_capacity;
            m_buffer = buffer.m_buffer;

            buffer.m_offset = 0;
            buffer.m_length = 0;
            buffer.m_capacity = 0;
            buffer.m_buffer = nullptr;
        }

        DataBuffer& DataBuffer::operator =(DataBuffer&& buffer) noexcept
        {
            m_offset = buffer.m_offset;
            m_length = buffer.m_length;
            m_capacity = buffer.m_capacity;
            m_buffer = buffer.m_buffer;

            buffer.m_offset = 0;
            buffer.m_length = 0;
            buffer.m_capacity = 0;
            buffer.m_buffer = nullptr;
            return *this;
        }

        DataBuffer& DataBuffer::Read(void* lp, size_t length)
        {
            if (m_offset + length > m_length)
                throw std::exception("EOF trying to read outside stream");

            memcpy(lp, m_buffer + m_offset, length);
            m_offset += length;
            return *this;
        }

        DataBuffer& DataBuffer::Write(const void* lp, size_t length)
        {
            EnsureWrite(length);

            memcpy(m_buffer + m_offset, lp, length);
            m_offset += length;
            if (m_length < m_offset)
                m_length = m_offset;

            return *this;
        }

        std::string DataBuffer::ReadString()
        {
            int length;
            *this >> length;

            thread_local char buf[4096];
            char* p = buf;
            if (length + 1 > sizeof(buf))
                p = (char*)malloc(length + 1);

            Read(p, length);

            std::string result(p, length);

            if (p != buf)
                free(p);

            return result;
        }

        DataBuffer& DataBuffer::WriteString(const std::string& str)
        {
            *this << (int)str.length();
            return Write(str.data(), str.length());
        }

        DataBuffer& DataBuffer::Delete(size_t count)
        {
            if (count > m_length)
                throw std::exception("EOF while trying to delete bytes");

            memmove(m_buffer, m_buffer + count, m_length - count);
            m_length -= count;
            if (m_offset >= count)
                m_offset -= count;
            else
                m_offset = 0;

            return *this;
        }

        void DataBuffer::EnsureWrite(size_t add)
        {
            if (m_offset + add <= m_capacity)
                return;

            size_t newCapacity = m_capacity;
            if (newCapacity == 0)
            {
                newCapacity = 16384;
            }

            while (newCapacity < m_offset + add)
                newCapacity *= 2;

            if (m_buffer)
                m_buffer = (char*)realloc(m_buffer, newCapacity);
            else
                m_buffer = (char*)malloc(newCapacity);

            m_capacity = newCapacity;
        }
    }
}