#include "StdAfx.h"

#include <NativeLib/Network/AsynchronousTcpServer.h>
#include <NativeLib/Platform/Platform.h>
#include <NativeLib/Allocators.h>

//!ALLOW_INCLUDE "Windows.h"

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace nl::network
{
    struct WorkerThreadParameters
    {
        AsynchronousTcpServer* pServer;
        uint32_t(AsynchronousTcpServer::*pfnWorkerThread)();
    };

    struct TcpServerState
    {
        HANDLE hWorkerThread;
        SOCKET Socket;

        TcpServerState()
        {
            hWorkerThread = nullptr;
            Socket = INVALID_SOCKET;
        }

        ~TcpServerState()
        {
            if (Socket != INVALID_SOCKET)
            {
                closesocket(Socket);
            }
        }
    };

#ifdef NL_PLATFORM_WINDOWS
    static DWORD _WorkerThread(LPVOID lp)
    {
        WorkerThreadParameters* parameters = static_cast<WorkerThreadParameters*>(lp);
        return (DWORD)(parameters->pServer->*parameters->pfnWorkerThread)();
    }
#endif

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AsynchronousTcpServer::AsynchronousTcpServer()
    {
        m_thread_parameters = nl::memory::ConstructThrow<WorkerThreadParameters>();
        m_thread_parameters->pServer = this;
        m_thread_parameters->pfnWorkerThread = &AsynchronousTcpServer::WorkerThread;

        m_state = nl::memory::ConstructThrow<TcpServerState>();
    }

    AsynchronousTcpServer::~AsynchronousTcpServer()
    {
        nl::memory::Destroy(m_state);
        nl::memory::Destroy(m_thread_parameters);
    }

    void AsynchronousTcpServer::Start(uint32_t bind_ip, uint16_t port, int32_t backlog, int32_t thread_count)
    {
        if (m_state->Socket != INVALID_SOCKET)
        {
            throw InvalidOperationException("Server already started");
        }

        m_state->Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_state->Socket == INVALID_SOCKET)
        {
            throw SocketException("Failed to create socket.", WSAGetLastError());
        }

        SOCKADDR_IN addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = bind_ip;
        addr.sin_port = htons(port);
        if (bind(m_state->Socket, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            closesocket(m_state->Socket);
            m_state->Socket = INVALID_SOCKET;
            throw SocketException("Failed to bind socket.", WSAGetLastError());
        }

        if (listen(m_state->Socket, backlog) == SOCKET_ERROR)
        {
            closesocket(m_state->Socket);
            m_state->Socket = INVALID_SOCKET;
            throw SocketException("Failed to listen on socket.", WSAGetLastError());
        }

        m_state->hWorkerThread = CreateThread(nullptr, 0, _WorkerThread, m_thread_parameters, 0, nullptr);
    }

    void AsynchronousTcpServer::Stop()
    {
        if (m_state->Socket == INVALID_SOCKET)
        {
            throw InvalidOperationException("Server is not running");
        }
    }

    uint32_t AsynchronousTcpServer::GetClientIP(DPID dpid)
    {
        return 0;
    }

    bool AsynchronousTcpServer::GetClientIP(DPID dpid, char* ptr)
    {
        return false;
    }

    void AsynchronousTcpServer::Disconnect(DPID dpid)
    {
    }

    void AsynchronousTcpServer::Send(DPID dpid, const void* lp, size_t len)
    {
    }

    uint32_t AsynchronousTcpServer::WorkerThread()
    {
        // TODO: event signals and stuff...

        //for (;;)
        //{            
        //}

        return 0;
    }
}