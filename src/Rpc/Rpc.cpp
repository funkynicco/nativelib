#include "StdAfx.h"

#ifdef NL_PLATFORM_WINDOWS

#include <NativeLib/Rpc.h>
#include <NativeLib/Logger.h>

#include "DataBuffer.h"
#include "RpcInternal.h"

#include <NativeLib/SystemLayer/SystemLayer.h>
#include <NativeLib/Allocators.h>
#include <NativeLib/Threading/Interlocked.h>
#include <NativeLib/RAII/Scoped.h>
#include <NativeLib/RAII/Shared.h>

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#undef GetMessage
#endif

#define HEADER_SIZE 8

namespace nl
{
    namespace rpc
    {
        static nl::Stack<OverlappedEx*> g_overlappedStack;

        inline OverlappedEx* AllocateOverlapped(IOEVENT event)
        {
            if (g_overlappedStack.GetCount() == 0)
                g_overlappedStack.Push((OverlappedEx*)nl::systemlayer::GetSystemLayerFunctions()->AllocateHeapMemory(sizeof(OverlappedEx)));

            auto lpOverlapped = g_overlappedStack.Pop();

            ZeroMemory(lpOverlapped, sizeof(OverlappedEx));
            lpOverlapped->Event = event;
            return lpOverlapped;
        }

        inline void FreeOverlapped(OverlappedEx* lpOverlapped)
        {
            g_overlappedStack.Push(lpOverlapped);
        }

        ///////////////

        Server::Server()
        {
            m_pfnEventHandler = nullptr;
            m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
            m_lUserData = 0;
            m_lNextClientId = 0;
        }

        Server::~Server()
        {
        }

        void Server::ConnectNewClient(const wchar_t* pipeName)
        {
            LONG lClientId = nl::threading::Interlocked::Increment(&m_lNextClientId);

            auto hPipe = CreateNamedPipe(
                pipeName,
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES,
                8192,
                8192,
                0,
                nullptr);

            CreateIoCompletionPort(hPipe, m_hIocp, 0, 0);

            auto lpOverlapped = AllocateOverlapped(IOEVENT_CONNECT);
            lpOverlapped->Client = nl::memory::ConstructThrow<PipeClient>(hPipe, lClientId);

            if (!ConnectNamedPipe(hPipe, &lpOverlapped->Overlapped) &&
                GetLastError() != ERROR_PIPE_CONNECTED &&
                GetLastError() != ERROR_IO_PENDING)
            {
                char msg[256];
                sprintf_s(msg, "Failed to connect named pipe, code: %u", GetLastError());
                nl::logger::GetGlobalLogger()->Log(nl::LogType::Error, __FILE__, __LINE__, __FUNCTION__, msg);

                lpOverlapped->Client->Release();
                FreeOverlapped(lpOverlapped);
            }
        }

        void Server::Run(const wchar_t* pipeName)
        {
            ConnectNewClient(pipeName);

            DWORD dwBytesTransferred;
            ULONG_PTR ulCompletionKey;
            OverlappedEx* lpOverlapped;

            for (;;)
            {
                if (!GetQueuedCompletionStatus(m_hIocp, &dwBytesTransferred, &ulCompletionKey, (LPOVERLAPPED*)&lpOverlapped, INFINITE))
                {
                    DWORD dwCode = GetLastError();
                    if (dwCode == ERROR_BROKEN_PIPE)
                    {
                        //DBG(__FUNCTION__ L" - %p disconnected\n", lpOverlapped->Client->GetPipe());
                        auto id = lpOverlapped->Client->GetId();
                        lpOverlapped->Client->Release();
                        FreeOverlapped(lpOverlapped);

                        if (m_pfnEventHandler)
                            m_pfnEventHandler(this, Events::ClientDisconnected, id);
                    }
                    else
                    {
                        char msg[256];
                        sprintf_s(msg, "GetQueuedCompletionStatus failed with code: %u\n", dwCode);
                        nl::logger::GetGlobalLogger()->Log(nl::LogType::Error, __FILE__, __LINE__, __FUNCTION__, msg);
                    }

                    continue;
                }

                if (lpOverlapped->Event == IOEVENT_SHUTDOWN)
                {
                    FreeOverlapped(lpOverlapped);
                    break;
                }

                if (lpOverlapped->Event == IOEVENT_CONNECT)
                {
                    HANDLE hPipe = lpOverlapped->Client->GetPipe();
                    auto id = lpOverlapped->Client->GetId();
                    //DBG(__FUNCTION__ L" - %p connected\n", hPipe);

                    lpOverlapped->Event = IOEVENT_READ;
                    ReadFile(hPipe, lpOverlapped->Buffer, sizeof(lpOverlapped->Buffer), nullptr, &lpOverlapped->Overlapped);
                    ConnectNewClient(pipeName);

                    if (m_pfnEventHandler)
                        m_pfnEventHandler(this, Events::ClientConnected, id);
                }
                else if (lpOverlapped->Event == IOEVENT_READ)
                {
                    //DBG(__FUNCTION__ L" - read %u bytes from %p\n", dwBytesTransferred, lpOverlapped->Client->GetPipe());

                    auto client = lpOverlapped->Client;
                    DataBuffer& buffer = client->GetBuffer();

                    buffer.SetOffset(buffer.GetLength());
                    buffer.Write(lpOverlapped->Buffer, dwBytesTransferred);

                    while (buffer.GetLength() >= HEADER_SIZE)
                    {
                        buffer.SetOffset(0);

                        int packetSize, requestId;
                        buffer >> packetSize >> requestId;

                        auto remaining = int(buffer.GetLength() - buffer.GetOffset());
                        if (remaining < packetSize)
                            break;

                        // dispatch and read packet...
                        HandleRequest(client, buffer, requestId);

                        buffer.Delete(HEADER_SIZE + packetSize);
                    }

                    ReadFile(lpOverlapped->Client->GetPipe(), lpOverlapped->Buffer, sizeof(lpOverlapped->Buffer), nullptr, &lpOverlapped->Overlapped);
                }
                else if (lpOverlapped->Event == IOEVENT_WRITE)
                {
                    //DBG(__FUNCTION__ L" - wrote %u bytes to %p\n", dwBytesTransferred, lpOverlapped->Client->GetPipe());
                    lpOverlapped->Client->Release();
                    FreeOverlapped(lpOverlapped);
                }
            }
        }

        void Server::HandleRequest(class PipeClient* client, class DataBuffer& buffer, int requestId)
        {
            const auto method = buffer.ReadString();
            const auto jsonDataString = buffer.ReadString();

            const auto& it = m_procedures.find(method);
            if (it == m_procedures.end())
            {
                SendError(client, requestId, "Method not defined.");
                return;
            }

            // Parse the request json data
            nl::Vector<nl::String> parse_errors;
            auto xb = nl::ParseJson(jsonDataString.c_str(), parse_errors);
            if (!xb ||
                xb->GetType() != nl::JsonType::Object)
            {
                SendError(client, requestId, "Invalid request JSON.");
                return;
            }

            auto requestJson = nl::Shared<nl::JsonObject>::Cast(xb);
            auto responseJson = nl::CreateJsonObject<nl::JsonObject>();

            try
            {
                it->second(this, client->GetId(), requestJson, responseJson);
            }
            catch (Exception & ex)
            {
                SendError(client, requestId, ex.GetMessage());
                return;
            }

            // Send back result ...
            auto resp = nl::CreateJsonObject<nl::JsonObject>();
            resp->SetNull("error");
            resp->SetObject("result", responseJson);
            SendJson(client, requestId, resp);
        }

        void Server::SendError(class PipeClient* client, int requestId, const char* message)
        {
            auto obj = nl::CreateJsonObject<nl::JsonObject>();
            obj->SetString("error", message);
            SendJson(client, requestId, obj);
        }

        void Server::SendJson(class PipeClient* client, int requestId, nl::Shared<const nl::JsonObject> json)
        {
            nl::String str;
            nl::GenerateJsonString(str, json);

            DataBuffer out;
            out << 0 << requestId; // packet size and request id
            out.WriteString(str);
            out.SetOffset(0);
            out << int(out.GetLength() - HEADER_SIZE);

            auto lpOverlapped = AllocateOverlapped(IOEVENT_WRITE);
            memcpy(lpOverlapped->Buffer, out.GetData(), out.GetLength());
            lpOverlapped->Client = client;
            lpOverlapped->Client->AddRef();
            WriteFile(client->GetPipe(), lpOverlapped->Buffer, (DWORD)out.GetLength(), nullptr, &lpOverlapped->Overlapped);
        }
    }
}

#endif