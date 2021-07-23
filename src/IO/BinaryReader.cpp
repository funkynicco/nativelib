#include "StdAfx.h"

#include <NativeLib/IO/BinaryStream.h>

namespace nl
{
    namespace io
    {
        BinaryReader::BinaryReader(Stream* stream) :
            m_stream(stream)
        {
        }

        Stream* BinaryReader::GetStream()
        {
            return m_stream;
        }

        int64_t BinaryReader::Read7BitEncodedInt()
        {
            int64_t value = 0;

            for (;;)
            {
                unsigned char by = 0;
                if (m_stream->Read(&by, 1) == 0)
                    throw IOException(IOException::ReadFailed);

                value <<= 7;
                value |= by & 0x7f;

                if ((by & 0x80) == 0)
                    break;
            }

            return value;
        }

        String BinaryReader::ReadString()
        {
            int64_t string_length = Read7BitEncodedInt();
            if (string_length == 0)
                return String();

            String str;
            str.SetLength(string_length);
            
            char* p = str.data();
            const char* end = p + string_length;
            while (p < end)
            {
                int64_t read = m_stream->Read(p, int64_t(end - p));
                if (read == 0)
                    throw IOException(IOException::ReadFailed);

                p += read;
            }

            return str;
        }
    }
}