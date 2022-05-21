#pragma once

#include <NativeLib/Network/NetworkCommon.h>

namespace nl::network
{
    class AsynchronousTcpServer
    {
    public:
        AsynchronousTcpServer();
        virtual ~AsynchronousTcpServer();

        void Start(uint32_t bind_ip, uint16_t port, int32_t backlog, int32_t thread_count);
        void Stop();

        uint32_t GetClientIP(DPID dpid);
        bool GetClientIP(DPID dpid, char* ptr);
        void Disconnect(DPID dpid);
        void Send(DPID dpid, const void* lp, size_t len);

    protected:
        virtual void OnClientConnected(nl::network::DPID dpId) {}
        virtual void OnClientDisconnected(nl::network::DPID dpId) {}
        virtual void OnClientDataReceived(nl::network::DPID dpId, const void* lp, size_t size) {}
        virtual void OnSendCompleted(nl::network::DPID dpId) {}

    private:
        uint32_t WorkerThread();
        
        struct WorkerThreadParameters* m_thread_parameters;
        friend struct TcpServerState;
        struct TcpServerState* m_state;
    };
}