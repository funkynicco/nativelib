#pragma once

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
            PipeClient(HANDLE hPipe) :
                m_hPipe(hPipe),
                m_lReferences(1)
            {
            }

            ~PipeClient()
            {
                CloseHandle(m_hPipe);
            }

            void AddRef()
            {
                InterlockedIncrement(&m_lReferences);
            }

            void Release()
            {
                if (InterlockedDecrement(&m_lReferences) == 0)
                    delete this;
            }

            HANDLE GetPipe() const { return m_hPipe; }
            DataBuffer& GetBuffer() { return m_buffer; }

        private:
            LONG m_lReferences;
            HANDLE m_hPipe;
            DataBuffer m_buffer;
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