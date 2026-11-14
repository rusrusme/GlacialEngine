//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <chrono>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <string>

namespace engine {
    struct ProfileResult {
        std::string name;
        long long start, end;
        uint32_t thread_id;
    };

    class Instrumentor {
    public:
        Instrumentor() = default;
        ~Instrumentor();

        Instrumentor(const Instrumentor&) = delete;
        Instrumentor& operator=(const Instrumentor&) = delete;

        static Instrumentor& get();

        void begin_session(const std::string& name, std::string filepath = "");
        void end_session();

        void write_profile(const ProfileResult& result);

    private:
        void write_header();
        void write_footer();

        void internal_end_session();

        std::ofstream out_stream_;
        int profile_count_ = 0;
        bool active_session_ = false;
        std::mutex mutex_;
    };

    class InstrumentationTimer {
    public:
        InstrumentationTimer(const char* name);
        ~InstrumentationTimer();

        void stop();

    private:
        const char* name_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint_;
        bool stopped_;
    };

    void begin_manual_section(const char* name);
    void end_manual_section();
}

#define ENABLE_PROFILER 1
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
