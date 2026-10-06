//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace engine {
    struct ProfileResult {
        std::string name;
        long long start, end;
        uint32_t thread_id;
    };

    struct ProfileStartInfo {
        const char* name;
        std::chrono::high_resolution_clock::time_point start_time;
    };

    class Instrumentor {
    public:
        Instrumentor() = default;
        ~Instrumentor() { end_session(); }

        Instrumentor(const Instrumentor&) = delete;
        Instrumentor& operator=(const Instrumentor&) = delete;

        static Instrumentor& get() {
            static Instrumentor instance;
            return instance;
        }

        void begin_session(const std::string& name, std::string filepath = "") {
            std::lock_guard lock(mutex_);
            if (active_session_) {
                internal_end_session();
            }

            if (filepath.empty()) {
                const auto now = std::chrono::system_clock::now();
                auto time_t_now = std::chrono::system_clock::to_time_t(now);

                std::tm local_tm{};
                #if defined(_MSC_VER) || defined(_WIN32)
                localtime_s(&local_tm, &time_t_now);
                #else
                localtime_r(&time_t_now, &local_tm);
                #endif
                std::stringstream ss;
                ss << "logs/profiler_" << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S") << ".json";
                filepath = ss.str();
            }

            const std::filesystem::path path(filepath);
            if (path.has_parent_path()) {
                std::filesystem::create_directories(path.parent_path());
            }

            out_stream_.open(filepath);
            if (out_stream_.is_open()) {
                active_session_ = true;
                write_header();
            }
        }

        void end_session() {
            std::lock_guard lock(mutex_);
            internal_end_session();
        }

        void write_profile(const ProfileResult& result) {
            std::lock_guard lock(mutex_);
            if (!active_session_ || !out_stream_.is_open()) return;

            if (profile_count_++ > 0) out_stream_ << ",\n";

            std::string name;
            name.reserve(result.name.size() * 2);
            for (const char c : result.name) {
                if (c == '"') name += "\\\"";
                else if (c == '\\') name += "\\\\";
                else if (c == '\n') name += "\\n";
                else if (c == '\r') name += "\\r";
                else if (c == '\t') name += "\\t";
                else name += c;
            }

            out_stream_ << "{"
                << "\"cat\":\"function\","
                << "\"dur\":" << (result.end - result.start) << ","
                << "\"name\":\"" << name << "\","
                << "\"ph\":\"X\","
                << "\"pid\":0,"
                << "\"tid\":" << result.thread_id << ","
                << "\"ts\":" << result.start
                << "}";
        }

    private:
        void write_header() {
            out_stream_ << "{\"otherData\": {},\"traceEvents\":[\n";
            out_stream_.flush();
        }

        void write_footer() {
            out_stream_ << "\n]}";
            out_stream_.flush();
        }

        void internal_end_session() {
            if (active_session_) {
                write_footer();
                out_stream_.flush();
                out_stream_.close();
                active_session_ = false;
                profile_count_ = 0;
            }
        }

        std::ofstream out_stream_;
        int profile_count_ = 0;
        bool active_session_ = false;
        std::mutex mutex_;
    };

    class InstrumentationTimer {
    public:
        InstrumentationTimer(const char* name) : name_(name), stopped_(false) {
            start_timepoint_ = std::chrono::high_resolution_clock::now();
        }

        ~InstrumentationTimer() {
            if (!stopped_) stop();
        }

        void stop() {
            if (stopped_) return;

            const auto end_timepoint = std::chrono::high_resolution_clock::now();

            const long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint_).time_since_epoch().count();
            const long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch().count();

            const uint32_t thread_id = static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            Instrumentor::get().write_profile({name_, start, end, thread_id});

            stopped_ = true;
        }

    private:
        const char* name_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint_;
        bool stopped_;
    };

    // Shared thread-local stack
    inline thread_local std::vector<ProfileStartInfo> g_manual_profile_stack;

    inline void begin_manual_section(const char* name) {
        g_manual_profile_stack.push_back({
            .name       = name,
            .start_time = std::chrono::high_resolution_clock::now()
        });
    }

    inline void end_manual_section() {
        if (g_manual_profile_stack.empty()) return;

        auto [name, start_time] = g_manual_profile_stack.back();
        g_manual_profile_stack.pop_back();

        const auto end_time = std::chrono::high_resolution_clock::now();
        const long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time).time_since_epoch().count();
        const long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();
        const uint32_t thread_id = static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

        Instrumentor::get().write_profile({name, start, end, thread_id});
    }
}

#ifdef ENABLE_PROFILER
#define PROFILE_BEGIN_SESSION(name, ...) ::engine::Instrumentor::get().begin_session(name, ##__VA_ARGS__)
#define PROFILE_END_SESSION()            ::engine::Instrumentor::get().end_session()
#define PROFILE_SCOPE(name)              ::engine::InstrumentationTimer timer##__LINE__(name)
#define PROFILE_FUNCTION()               PROFILE_SCOPE(__FUNCTION__)

#define PROFILE_START(name) ::engine::begin_manual_section(name)
#define PROFILE_END()       ::engine::end_manual_section()
#else
#define PROFILE_BEGIN_SESSION(name, ...)
#define PROFILE_END_SESSION()
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#define PROFILE_START(name)
#define PROFILE_END()
#endif
