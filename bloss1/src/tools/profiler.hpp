#pragma once

#include "core/logger.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>

namespace bls
{
    using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

    struct ProfileResult
    {
        std::string Name;

        FloatingPointMicroseconds Start;
        std::chrono::microseconds ElapsedTime;
        std::thread::id ThreadID;
    };

    struct ProfilerSession
    {
        std::string Name;
    };

    class Profiler
    {
        public:
            Profiler(const Profiler&) = delete;
            Profiler(Profiler&&) = delete;

            void BeginSession(const std::string& name, const std::string& filepath = "results.json")
            {
                std::lock_guard lock(m_Mutex);
                if (m_CurrentSession)
                {
                    LOG_ERROR("BeginSession('%s') when session '%s' already open.",
                              name.c_str(), m_CurrentSession->Name.c_str());

                    InternalEndSession();
                }
                m_OutputStream.open(filepath);

                if (m_OutputStream.is_open())
                {
                    m_CurrentSession = new ProfilerSession({name});
                    WriteHeader();
                }

                else
                    LOG_ERROR("Profiler could not open results file '%s'.", filepath.c_str());
            }

            void EndSession()
            {
                std::lock_guard lock(m_Mutex);
                InternalEndSession();
            }

            void WriteProfile(const ProfileResult& result)
            {
                std::stringstream json;

                json << std::setprecision(3) << std::fixed;
                json << ",{";
                json << "\"cat\":\"function\",";
                json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
                json << "\"name\":\"" << result.Name << "\",";
                json << "\"ph\":\"X\",";
                json << "\"pid\":0,";
                json << "\"tid\":" << result.ThreadID << ",";
                json << "\"ts\":" << result.Start.count();
                json << "}";

                std::lock_guard lock(m_Mutex);
                if (m_CurrentSession)
                {
                    m_OutputStream << json.str();
                    m_OutputStream.flush();
                }
            }

            static Profiler& Get()
            {
                static Profiler instance;
                return instance;
            }

        private:
            Profiler()
                : m_CurrentSession(nullptr) { }

            ~Profiler() { EndSession(); }

            void WriteHeader()
            {
                m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
                m_OutputStream.flush();
            }

            void WriteFooter()
            {
                m_OutputStream << "]}";
                m_OutputStream.flush();
            }

            void InternalEndSession()
            {
                if (m_CurrentSession)
                {
                    WriteFooter();
                    m_OutputStream.close();
                    delete m_CurrentSession;
                    m_CurrentSession = nullptr;
                }
            }

            std::mutex m_Mutex;
            ProfilerSession* m_CurrentSession;
            std::ofstream m_OutputStream;
    };

    class ProfilerTimer
    {
        public:
            ProfilerTimer(const char* name)
                : m_Name(name), m_Stopped(false)
            {
                m_StartTimepoint = std::chrono::steady_clock::now();
            }

            ~ProfilerTimer()
            {
                if (!m_Stopped)
                    Stop();
            }

            void Stop()
            {
                auto endTimepoint = std::chrono::steady_clock::now();
                auto highResStart = FloatingPointMicroseconds{ m_StartTimepoint.time_since_epoch() };
                auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

                Profiler::Get().WriteProfile({ m_Name, highResStart, elapsedTime, std::this_thread::get_id() });

                m_Stopped = true;
            }

        private:
            const char* m_Name;
            std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
            bool m_Stopped;
    };

    namespace profiler_utils
    {
        template <size_t N>
        struct ChangeResult
        {
            char Data[N];
        };

        template <size_t N, size_t K>
        constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
        {
            ChangeResult<N> result = {};

            size_t srcIndex = 0;
            size_t dstIndex = 0;
            while (srcIndex < N)
            {
                size_t matchIndex = 0;
                while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
                    matchIndex++;
                if (matchIndex == K - 1)
                    srcIndex += matchIndex;
                result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
                srcIndex++;
            }
            return result;
        }
    }
}

#ifdef BLS_PROFILE
    #define BLS_PROFILE_BEGIN_SESSION(name, filepath) bls::Profiler::Get().BeginSession(name, filepath)
    #define BLS_PROFILE_END_SESSION()                 bls::Profiler::Get().EndSession()
    #define BLS_PROFILE_SCOPE(name)                   bls::ProfilerTimer timer##__LINE__(name);
    #define BLS_PROFILE_FUNCTION() BLS_PROFILE_SCOPE(__FUNCTION__)
#else
    #define BLS_PROFILE_BEGIN_SESSION(name, filepath)
    #define BLS_PROFILE_END_SESSION()
    #define BLS_PROFILE_SCOPE(name)
    #define BLS_PROFILE_FUNCTION()
#endif
