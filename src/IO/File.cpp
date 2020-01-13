#include "StdAfx.h"

#include <NativeLib/IO/File.h>

namespace nl
{
    namespace io
    {
        File::File() :
            m_fp(0)
        {
        }

        File::File(systemlayer::FileHandle fp) :
            m_fp(fp)
        {
        }

        File::~File()
        {
            Close();
        }

        File::operator bool() const
        {
            return m_fp != 0;
        }

        bool File::IsOpen() const
        {
            return m_fp != 0;
        }

        int64_t File::GetPosition() const
        {
            return systemlayer::GetSystemLayerFunctions()->FileGetPosition(m_fp);
        }

        int64_t File::GetSize() const
        {
            return systemlayer::GetSystemLayerFunctions()->FileGetSize(m_fp);
        }

        void File::Close()
        {
            if (m_fp != 0)
            {
                systemlayer::GetSystemLayerFunctions()->FileClose(m_fp);
                m_fp = 0;
            }
        }

        bool File::Seek(SeekMode mode, int64_t offset)
        {
            return systemlayer::GetSystemLayerFunctions()->FileSeek(m_fp, offset, mode);
        }

        size_t File::Read(void* lp, size_t count)
        {
            return systemlayer::GetSystemLayerFunctions()->FileRead(m_fp, lp, count);
        }

        size_t File::Write(const void* lp, size_t count)
        {
            return systemlayer::GetSystemLayerFunctions()->FileWrite(m_fp, lp, count);            
        }

        File File::Open(std::string_view filename, CreateMode mode, bool writable)
        {
            return File(systemlayer::GetSystemLayerFunctions()->FileOpen(nl::String(filename), mode, writable));
        }
    }
}