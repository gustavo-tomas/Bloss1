#pragma once

/**
 * @brief Simple formatted logging for the terminal.
 */

#include "config.hpp"
#include "core/core.hpp"
#include "imgui/imgui.h"
#include "math/math.hpp"

namespace bls
{
    enum class LogType
    {
        Error,
        Warning,
        Info,
        Success
    };

    struct Log
    {
            str message;
            LogType log_type;
            vec4 color;
            str timestamp;
    };

    class Logger
    {
        public:
            static void log(LogType log_type, const char* message, ...) IM_FMTARGS(2)
            {
                vec4 color = vec4(0.0f);
                str severity = "";
                switch (log_type)
                {
                    case LogType::Error:
                        severity = "[error]";
                        color = {1.0f, 0.0f, 0.0f, 1.0f};
                        break;

                    case LogType::Warning:
                        severity = "[warning]";
                        color = {1.0f, 1.0f, 0.0f, 1.0f};
                        break;

                    case LogType::Info:
                        severity = "[info]";
                        color = {1.0f, 1.0f, 1.0f, 1.0f};
                        break;

                    case LogType::Success:
                        severity = "[success]";
                        color = {0.0f, 1.0f, 0.0f, 1.0f};
                        break;

                    default:
                        break;
                }

                auto current_time = std::chrono::system_clock::now();
                std::time_t current_time_t = std::chrono::system_clock::to_time_t(current_time);
                std::tm* current_time_tm = std::localtime(&current_time_t);

                std::ostringstream oss;
                oss << std::put_time(current_time_tm, "%H:%M:%S");
                const str timestamp = oss.str();

                const str fmt = "[" + timestamp + "] " + severity + " " + message;
                ImGuiTextBuffer buf;

                va_list va_args;
                va_start(va_args, message);
                buf.appendfv(fmt.c_str(), va_args);
                va_end(va_args);

                AppStats::log_messages.push_back({buf.c_str(), log_type, color, timestamp});
            }
    };

#if !defined(_RELEASE)
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
