#pragma once

namespace nl
{
    enum class LogType
    {
        Debug,
        Info,
        Warn,
        Error,
        Critical,
    };

    struct ILogger
    {
        virtual void Log(
            LogType logType,
            const char* filename,
            int line,
            const char* function,
            const char* message) = 0;
    };

    namespace logger
    {
        void SetGlobalLogger(ILogger* logger);
        ILogger* GetGlobalLogger();
        void GetLogTypeName(LogType logType, char* name, size_t sizeOfName);
    }
}