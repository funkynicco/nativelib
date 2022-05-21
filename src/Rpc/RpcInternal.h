#pragma once

#ifdef NL_PLATFORM_WINDOWS

#include <NativeLib/Allocators.h>
#include <NativeLib/Threading/Interlocked.h>

//!ALLOW_INCLUDE "Windows.h"

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace nl
{
    namespace rpc
    {
        enum IOEVENT
        {
            IOEVENT_SHUTDOWN,
            IOEVENT_CONNECT,
            IOEVENT_READ,
            IOEVENT_WRITE
        };

        class PipeClient
        {
        public:
            PipeClient(HANDLE hPipe, int32_t lClientId) :
                m_hPipe(hPipe),
                m_lReferences(1),
                m_lClientId(lClientId)
            {
            }

            ~PipeClient()
            {
                CloseHandle(m_hPipe);
            }

            void AddRef()
            {
                nl::threading::Interlocked::Increment(&m_lReferences);
            }

            void Release()
            {
                if (nl::threading::Interlocked::Decrement(&m_lReferences) == 0)
                    nl::memory::Destroy(this);
            }

            HANDLE GetPipe() const { return m_hPipe; }
            DataBuffer& GetBuffer() { return m_buffer; }
            LONG GetId() const { return m_lClientId; }

        private:
            int32_t m_lReferences;
            void* m_hPipe;
            DataBuffer m_buffer;
            int32_t m_lClientId;
        };

        struct OverlappedEx
        {
            OVERLAPPED Overlapped;
            IOEVENT Event;
            PipeClient* Client;
            char Buffer[65536];
        };
    }
}

#endif