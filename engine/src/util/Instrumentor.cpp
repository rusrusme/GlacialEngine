//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "util/Instrumentor.hpp"

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

namespace engine {
    namespace {
        struct ProfileStartInfo {
            const char* name{};
            std::chrono::high_resolution_clock::time_point start_time;
        };

        thread_local std::vector<ProfileStartInfo> g_manual_profile_stack;
    }

    Instrumentor::~Instrumentor() { end_session(); }

    Instrumentor& Instrumentor::get() {
        static Instrumentor instance;
        return instance;
    }

    void Instrumentor::begin_session(const std::string& name, std::string filepath) {
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

    void Instrumentor::end_session() {
        std::lock_guard lock(mutex_);
        internal_end_session();
    }

    void Instrumentor::write_profile(const ProfileResult& result) {
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

    void Instrumentor::write_header() {
        out_stream_ << "{\"otherData\": {},\"traceEvents\":[\n";
        out_stream_.flush();
    }

    void Instrumentor::write_footer() {
        out_stream_ << "\n]}";
        out_stream_.flush();
    }

    void Instrumentor::internal_end_session() {
        if (active_session_) {
            write_footer();
            out_stream_.flush();
            out_stream_.close();
            active_session_ = false;
            profile_count_ = 0;
        }
    }

    InstrumentationTimer::InstrumentationTimer(const char* name) : name_(name), stopped_(false) {
        start_timepoint_ = std::chrono::high_resolution_clock::now();
    }

    InstrumentationTimer::~InstrumentationTimer() {
        if (!stopped_) stop();
    }

    void InstrumentationTimer::stop() {
        if (stopped_) return;

        const auto end_timepoint = std::chrono::high_resolution_clock::now();

        const long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint_).time_since_epoch().count();
        const long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch().count();

        const uint32_t thread_id = static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        Instrumentor::get().write_profile({name_, start, end, thread_id});

        stopped_ = true;
    }

    void begin_manual_section(const char* name) {
        g_manual_profile_stack.push_back({
            .name       = name,
            .start_time = std::chrono::high_resolution_clock::now()
        });
    }

    void end_manual_section() {
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
