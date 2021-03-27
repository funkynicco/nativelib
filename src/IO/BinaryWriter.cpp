#include "StdAfx.h"

#include <NativeLib/IO/BinaryStream.h>

namespace nl
{
    namespace io
    {
        BinaryWriter::BinaryWriter(Stream* stream) :
            m_stream(stream)
        {
        }

        Stream* BinaryWriter::GetStream()
        {
            return m_stream;
        }

        void BinaryWriter::Write7BitEncodedInt(int64_t value)
        {
            // convert to unsigned to support negative values
            size_t val = (size_t)value;

            // a 64 bit integer can span 10 bytes due to 7bit encoding
            const size_t BufferSize = 10;
            unsigned char buffer[BufferSize];

            size_t i = BufferSize - 1;

            buffer[i] = val & 0x7f;
            val >>= 7;

            while (val != 0)
            {
                buffer[i] |= 0x80;
                buffer[--i] = val & 0x7f;
                val >>= 7;
            }

            const unsigned char* p = buffer;
            const unsigned char* end = p + (BufferSize - i);
            while (p < end)
            {
                int64_t written = m_stream->Write(p, int64_t(end - p));
                if (written == 0)
                    throw IOException(IOException::WriteFailed);

                p += written;
            }
        }

        void BinaryWriter::WriteString(std::string_view value)
        {
            Write7BitEncodedInt(value.length());

            const char* p = value.data();
            const char* end = p + value.length();
            while (p < end)
            {
                int64_t written = m_stream->Write(p, int64_t(end - p));
                if (written == 0)
                    throw IOException(IOException::WriteFailed);

                p += written;
            }
        }
    }
}