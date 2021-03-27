#pragma once

#include <NativeLib/IO/Stream.h>
#include <NativeLib/String.h>

// For the future:
// - Read and write 7 bit encoded compressed integer

namespace nl
{
    namespace io
    {
        class BinaryReader
        {
        public:
            BinaryReader(Stream* stream);

            Stream* GetStream();

            template <typename T>
            BinaryReader& operator >>(T& value)
            {
                char* p = reinterpret_cast<char*>(&value);
                const char* end = p + sizeof(T);
                while (p < end)
                {
                    int64_t read = m_stream->Read(p, int64_t(end - p));
                    if (read == 0)
                        throw IOException(IOException::ReadFailed);

                    p += read;
                }

                return *this;
            }

            int64_t Read7BitEncodedInt();

            // Reads a string that is prefixed with 7bit encoded integer length (using Read7BitEncodedInt)
            String ReadString();

        private:
            Stream* m_stream;
        };

        /////////////////////////////////////////////////////
        /////////////////////////////////////////////////////
        /////////////////////////////////////////////////////

        class BinaryWriter
        {
        public:
            BinaryWriter(Stream* stream);

            Stream* GetStream();

            template <typename T>
            BinaryWriter& operator <<(const T& value)
            {
                const char* p = reinterpret_cast<const char*>(&value);
                const char* end = p + sizeof(T);
                while (p < end)
                {
                    int64_t written = m_stream->Write(p, int64_t(end - p));
                    if (written == 0)
                        throw IOException(IOException::WriteFailed);

                    p += written;
                }

                return *this;
            }

            void Write7BitEncodedInt(int64_t value);

            // Writes a string that is prefixed with 7bit encoded integer length (using Write7BitEncodedInt)
            void WriteString(std::string_view value);

        private:
            Stream* m_stream;
        };
    }
}