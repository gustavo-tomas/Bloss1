#pragma once

/**
 * @brief Simple time profiler based on The Cherno implementation.
 */

#include "core/logger.hpp"

namespace bls
{
    using FloatingPointMicroseconds = std::chrono::duration<f64, std::micro>;

    struct ProfileResult
    {
            str name;

            FloatingPointMicroseconds start;
            std::chrono::microseconds elapsed_time;
            std::thread::id thread_id;
    };

    struct ProfilerSession
    {
            str name;
    };

    class Profiler
    {
        public:
            Profiler(const Profiler &) = delete;
            Profiler(Profiler &&) = delete;

            void begin_session(const str &name, const str &filepath = "results.json")
            {
                std::lock_guard lock(mutex);
                if (current_session)
                {
                    LOG_ERROR(
                        "begin_session('%s') when "
                        "session '%s' already open.",
                        name.c_str(),
                        current_session->name.c_str());

                    internal_end_session();
                }
                output_stream.open(filepath);

                if (output_stream.is_open())
                {
                    current_session = new ProfilerSession({name});
                    write_header();
                }

                else
                    LOG_ERROR(
                        "Profiler could not open results "
                        "file '%s'.",
                        filepath.c_str());
            }

            void end_session()
            {
                std::lock_guard lock(mutex);
                internal_end_session();
            }

            void write_profile(const ProfileResult &result)
            {
                std::stringstream json;

                json << std::setprecision(3) << std::fixed;
                json << ",{";
                json << "\"cat\":\"function\",";
                json << "\"dur\":" << (result.elapsed_time.count()) << ',';
                json << "\"name\":\"" << result.name << "\",";
                json << "\"ph\":\"X\",";
                json << "\"pid\":0,";
                json << "\"tid\":" << result.thread_id << ",";
                json << "\"ts\":" << result.start.count();
                json << "}";

                std::lock_guard lock(mutex);
                if (current_session)
                {
                    output_stream << json.str();
                    output_stream.flush();
                }
            }

            static Profiler &get()
            {
                static Profiler instance;
                return instance;
            }

        private:
            Profiler() : current_session(nullptr)
            {
            }

            ~Profiler()
            {
                end_session();
            }

            void write_header()
            {
                output_stream << "{\"otherData\": {},\"traceEvents\":[{}";
                output_stream.flush();
            }

            void write_footer()
            {
                output_stream << "]}";
                output_stream.flush();
            }

            void internal_end_session()
            {
                if (current_session)
                {
                    write_footer();
                    output_stream.close();
                    delete current_session;
                    current_session = nullptr;
                }
            }

            std::mutex mutex;
            ProfilerSession *current_session;
            std::ofstream output_stream;
    };

    class ProfilerTimer
    {
        public:
            ProfilerTimer(const char *name) : name(name), stopped(false)
            {
                start_timepoint = std::chrono::steady_clock::now();
            }

            ~ProfilerTimer()
            {
                if (!stopped) stop();
            }

            void stop()
            {
                auto end_timepoint = std::chrono::steady_clock::now();
                auto high_res_start = FloatingPointMicroseconds{start_timepoint.time_since_epoch()};
                auto elapsed_time =
                    std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch() -
                    std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint).time_since_epoch();

                Profiler::get().write_profile({name, high_res_start, elapsed_time, std::this_thread::get_id()});

                stopped = true;
            }

        private:
            const char *name;
            std::chrono::time_point<std::chrono::steady_clock> start_timepoint;
            bool stopped;
    };

    namespace profiler_utils
    {
        template <size_t N>
        struct ChangeResult
        {
                char data[N];
        };

        template <size_t N, size_t K>
        constexpr auto CleanupOutputString(const char (&expr)[N], const char (&remove)[K])
        {
            ChangeResult<N> result = {};

            size_t src_index = 0;
            size_t dst_index = 0;
            while (src_index < N)
            {
                size_t match_index = 0;
                while (match_index < K - 1 && src_index + match_index < N - 1 &&
                       expr[src_index + match_index] == remove[match_index])
                    match_index++;

                if (match_index == K - 1) src_index += match_index;

                result.data[dst_index++] = expr[src_index] == '"' ? '\'' : expr[src_index];
                src_index++;
            }

            return result;
        }
    }  // namespace profiler_utils
}  // namespace bls

#ifdef BLS_PROFILE
#define BLS_PROFILE_BEGIN_SESSION(name, filepath) bls::Profiler::get().begin_session(name, filepath)
#define BLS_PROFILE_END_SESSION() bls::Profiler::get().end_session()
#define BLS_PROFILE_SCOPE(name) bls::ProfilerTimer timer##__LINE__(name);
#define BLS_PROFILE_FUNCTION() BLS_PROFILE_SCOPE(__FUNCTION__)
#else
#define BLS_PROFILE_BEGIN_SESSION(name, filepath)
#define BLS_PROFILE_END_SESSION()
#define BLS_PROFILE_SCOPE(name)
#define BLS_PROFILE_FUNCTION()
#endif
