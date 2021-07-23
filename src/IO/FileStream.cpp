#include "StdAfx.h"

#include <NativeLib/IO/FileStream.h>

namespace nl
{
    namespace io
    {
        FileStream::FileStream() :
            m_fp(0)
        {
        }

        FileStream::FileStream(systemlayer::FileHandle fp) :
            m_fp(fp)
        {
        }

        FileStream::~FileStream()
        {
            Close();
        }

        FileStream::operator bool() const
        {
            return m_fp != 0;
        }

        bool FileStream::CanSeek() const
        {
            return true;
        }

        bool FileStream::CanRead() const
        {
            return true;
        }

        bool FileStream::CanWrite() const
        {
            return true;
        }

        bool FileStream::IsOpen() const
        {
            return m_fp != 0;
        }

        int64_t FileStream::GetPosition() const
        {
            return systemlayer::GetSystemLayerFunctions()->FileGetPosition(m_fp);
        }

        int64_t FileStream::GetLength() const
        {
            return systemlayer::GetSystemLayerFunctions()->FileGetSize(m_fp);
        }

        void FileStream::SetLength(int64_t length)
        {
            // save the current position to restore later
            int64_t pos = GetPosition();

            // seek to the position of which to become the new file size
            Seek(length, SeekMode::Begin);

            auto error = false;
            
            // set the end of the file at the current position
            if (!systemlayer::GetSystemLayerFunctions()->FileSetEndOfFile(m_fp))
                error = true;
            
            // restore position
            Seek(pos, SeekMode::Begin);

            if (error)
                throw IOException("SetLength failed");
        }

        void FileStream::Flush()
        {
            if (!systemlayer::GetSystemLayerFunctions()->FileFlush(m_fp))
            {
                throw IOException("Failed to flush file");
            }
        }

        void FileStream::Close()
        {
            if (m_fp != 0)
            {
                systemlayer::GetSystemLayerFunctions()->FileClose(m_fp);
                m_fp = 0;
            }
        }

        int64_t FileStream::Seek(int64_t offset, SeekMode mode)
        {
            if (!systemlayer::GetSystemLayerFunctions()->FileSeek(m_fp, offset, mode))
            {
                throw IOException(IOException::SeekFailed);
            }

            if (mode == SeekMode::Begin)
            {
                return offset;
            }

            return GetPosition();
        }

        int64_t FileStream::Read(void* lp, int64_t numberOfBytesToRead)
        {
            return systemlayer::GetSystemLayerFunctions()->FileRead(m_fp, lp, numberOfBytesToRead);
        }

        int64_t FileStream::Write(const void* lp, int64_t numberOfBytesToWrite)
        {
            return systemlayer::GetSystemLayerFunctions()->FileWrite(m_fp, lp, numberOfBytesToWrite);
        }

        FileStream FileStream::Open(std::string_view filename, CreateMode mode, bool writable)
        {
            return FileStream(systemlayer::GetSystemLayerFunctions()->FileOpen(nl::String(filename), mode, writable));
        }
    }
}