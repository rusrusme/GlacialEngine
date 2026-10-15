//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <format>
#include <mutex>
#include <string>
#include <vector>

namespace engine {
    enum class LogLevel { Trace, Info, Warn, Error, Critical };

    struct LogEntry {
        std::string message;
        LogLevel level;
        std::string timestamp;
    };

    class Log {
    public:
        static void init();

        template<typename... Args>
        static void log(LogLevel level, const char* category, std::format_string<Args...> fmt, Args&&... args);

        static const std::vector<LogEntry>& get_entries();
        static void clear();

    private:
        static void add_entry(LogLevel level, const char* category, const std::string& message);

        static inline std::vector<LogEntry> entries_;
        static inline std::mutex mutex_;
    };

    template<typename... Args>
    void Log::log(const LogLevel level, const char* category, std::format_string<Args...> fmt, Args&&... args) {
        const std::string formatted_msg = std::format(fmt, std::forward<Args>(args)...);
        add_entry(level, category, formatted_msg);
    }

    namespace util {
        // Copy
        std::string to_lowercase(std::string str);
        std::string to_uppercase(std::string str);
        // In-place
        void to_lowercase_inplace(std::string& str);
        void to_uppercase_inplace(std::string& str);
    }
}

// Core Engine Logging Macros
#define CORE_TRACE(...)    ::engine::Log::log(::engine::LogLevel::Trace,    "ENGINE ", __VA_ARGS__)
#define CORE_INFO(...)     ::engine::Log::log(::engine::LogLevel::Info,     "ENGINE ", __VA_ARGS__)
#define CORE_WARN(...)     ::engine::Log::log(::engine::LogLevel::Warn,     "ENGINE [WARN]", __VA_ARGS__)
#define CORE_ERROR(...)    ::engine::Log::log(::engine::LogLevel::Error,    "ENGINE [ERROR]", __VA_ARGS__)
#define CORE_CRITICAL(...) ::engine::Log::log(::engine::LogLevel::Critical, "ENGINE [CRITICAL]", __VA_ARGS__)

// Client Application Logging Macros
#define CLIENT_TRACE(...)    ::engine::Log::log(::engine::LogLevel::Trace,    "APP", __VA_ARGS__)
#define CLIENT_INFO(...)     ::engine::Log::log(::engine::LogLevel::Info,     "APP ", __VA_ARGS__)
#define CLIENT_WARN(...)     ::engine::Log::log(::engine::LogLevel::Warn,     "APP [WARN]", __VA_ARGS__)
#define CLIENT_ERROR(...)    ::engine::Log::log(::engine::LogLevel::Error,  , "APP [ERROR]", __VA_ARGS__)
#define CLIENT_CRITICAL(...) ::engine::Log::log(::engine::LogLevel::Critical, "APP [CRITICAL]", __VA_ARGS__)
