//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/Log.hpp"

namespace engine {
    void Log::init() {
        clear();
    }

    const std::vector<LogEntry>& Log::get_entries() {
        return entries_;
    }

    void Log::clear() {
        std::lock_guard lock(mutex_);
        entries_.clear();
    }

    void Log::add_entry(LogLevel level, const char* category, const std::string& message) {
        std::lock_guard lock(mutex_);

        const auto now = std::chrono::system_clock::now();
        const auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now{};
        #if defined(_WIN32)
        localtime_s(&tm_now, &time_t_now);
        #else
        localtime_r(&time_t_now, &tm_now);
        #endif

        std::ostringstream ss;
        ss << std::put_time(&tm_now, "%H:%M:%S");
        std::string time_str = ss.str();

        std::string formatted = std::format("[{}] {}: {}", time_str, category, message);
        std::cout << formatted << std::endl;

        entries_.push_back({.message = formatted, .level = level, .timestamp = time_str});
    }
}
