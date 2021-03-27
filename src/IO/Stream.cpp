#include "StdAfx.h"

#include <NativeLib/IO/Stream.h>

namespace nl
{
    namespace io
    {
        int64_t Stream::GetPosition() const
        {
            throw NotSupportedException("GetPosition is not supported.");
        }

        int64_t Stream::GetLength() const
        {
            throw NotSupportedException("GetLength is not supported.");
        }

        int64_t Stream::Seek(int64_t offset, SeekMode mode)
        {
            throw NotSupportedException("Seek is not supported.");
        }

        void Stream::SetLength(int64_t length)
        {
            throw NotSupportedException("SetLength is not supported.");
        }

        void Stream::Flush()
        {
            throw NotSupportedException("Flush is not supported.");
        }

        void Stream::Close()
        {
        }

        void Stream::CopyTo(Stream* stream)
        {
            if (!CanRead())
            {
                throw NotSupportedException("This stream cannot be read from.");
            }

            if (!stream->CanWrite())
            {
                throw ArgumentException("The provided stream cannot be written to.");
            }

            char buffer[8192];
            for (;;)
            {
                int64_t read = Read(buffer, sizeof(buffer));
                if (read == 0)
                    break;

                int64_t write_pos = 0;
                while (write_pos < read)
                {
                    int64_t written = stream->Write(buffer, read);
                    if (written == 0)
                        throw IOException("Failed to write to stream.");

                    write_pos += written;
                }
            }
        }
    }
}