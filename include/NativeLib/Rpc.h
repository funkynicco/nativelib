#pragma once

#ifdef NL_PLATFORM_WINDOWS

#include <NativeLib/Containers/Map.h>
#include <NativeLib/Containers/Stack.h>

#include <NativeLib/Json.h>
#include <NativeLib/Exceptions.h>
#include <NativeLib/String.h>
#include <NativeLib/RAII/Shared.h>

namespace nl
{
    namespace rpc
    {
        DeclarePassthroughException(RpcException);

        enum class Events
        {
            ClientConnected,
            ClientDisconnected
        };

        typedef void(*pfnEventHandler)(class Server* rpc, Events event, intptr_t data);
        typedef void(*pfn)(class Server* rpc, int32_t client_id, nl::Shared<const nl::JsonObject> request, nl::Shared<nl::JsonObject> response);

        class Server
        {
        public:
            Server();
            ~Server();
            void Run(const wchar_t* pipeName);

            void BindEventHandler(pfnEventHandler pfn)
            {
                m_pfnEventHandler = pfn;
            }

            void Bind(const nl::String& name, pfn procedure)
            {
                m_procedures.Add(name, procedure);
            }

            intptr_t GetUserData() { return m_lUserData; }
            void SetTag(intptr_t lUserData) { m_lUserData = lUserData; }

        protected:
            void HandleRequest(class PipeClient* client, class DataBuffer& buffer, int requestId);
            void SendError(class PipeClient* client, int requestId, const char* message);
            void SendJson(class PipeClient* client, int requestId, nl::Shared<const nl::JsonObject> json);

        private:
            pfnEventHandler m_pfnEventHandler;
            nl::Map<nl::String, pfn> m_procedures;
            void* m_hIocp;
            intptr_t m_lUserData;

            int32_t m_lNextClientId;

            void ConnectNewClient(const wchar_t* pipeName);
        };
    }
}

#endif