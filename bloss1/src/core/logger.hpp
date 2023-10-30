#pragma once

/**
 * @brief Simple formatted logging for the terminal.
 */

#include "core/core.hpp"

namespace bls
{
    enum class LogType
    {
        Error,
        Warning,
        Info,
        Success
    };

    class Logger
    {
        public:
            template <typename T, typename... Args>
            static void log(LogType log_type, T message, Args... args)
            {
                str color = "";
                str reset = "\033[0m";
                switch (log_type)
                {
                    case LogType::Error:
                        color = "\033[31;1m";
                        break;

                    case LogType::Warning:
                        color = "\033[33;1m";
                        break;

                    case LogType::Info:
                        color = "\033[37;1m";
                        break;

                    case LogType::Success:
                        color = "\033[32;1m";
                        break;

                    default:
                        break;
                }

                printf((color + static_cast<str>(message) + "\n" + reset).c_str(), args...);
            }
    };

#if defined(_DEBUG)
#define LOG_ERROR(message, ...) Logger::log(LogType::Error, message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...) Logger::log(LogType::Warning, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...) Logger::log(LogType::Info, message, ##__VA_ARGS__)
#define LOG_SUCCESS(message, ...) Logger::log(LogType::Success, message, ##__VA_ARGS__)
#else
#define LOG_ERROR(message, ...)
#define LOG_WARNING(message, ...)
#define LOG_INFO(message, ...)
#define LOG_SUCCESS(message, ...)
#endif
};  // namespace bls
