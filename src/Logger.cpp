#include "StdAfx.h"

#include <NativeLib/Logger.h>
#include <NativeLib/Assert.h>

namespace nl
{
    namespace logger
    {
        class DummyLogger : public ILogger
        {
        public:
            virtual void Log(
                LogType logType,
                const char* filename,
                int line,
                const char* function,
                const char* message) override
            {
            }
        };

        static DummyLogger g_dummyLogger;
        static ILogger* g_logger = &g_dummyLogger;

        void SetGlobalLogger(ILogger* logger)
        {
            nl_assert_if_debug(logger != nullptr);
            g_logger = logger;
        }

        ILogger* GetGlobalLogger()
        {
            return g_logger;
        }

        void GetLogTypeName(LogType logType, char* name, size_t sizeOfName)
        {
            nl_assert_if_debug(name != nullptr);
            nl_assert_if_debug(sizeOfName != 0);

            *name = 0;

            switch (logType)
            {
            case LogType::Debug: strcpy_s(name, sizeOfName, "Debug"); break;
            case LogType::Info: strcpy_s(name, sizeOfName, "Info"); break;
            case LogType::Warn: strcpy_s(name, sizeOfName, "Warn"); break;
            case LogType::Error: strcpy_s(name, sizeOfName, "Error"); break;
            case LogType::Critical: strcpy_s(name, sizeOfName, "Critical"); break;
            }
        }
    }
}