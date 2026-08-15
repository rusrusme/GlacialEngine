//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#ifndef DIST_BUILD
#include <spdlog/spdlog.h>
#include <memory>
#endif

namespace engine {
    class Log {
    public:
        static void init();
        #ifndef DIST_BUILD

        static std::shared_ptr<spdlog::logger>& get_core_logger() {
            return core_logger;
        }

        static std::shared_ptr<spdlog::logger>& get_client_logger() {
            return client_logger;
        }

    private:
        static std::shared_ptr<spdlog::logger> core_logger;
        static std::shared_ptr<spdlog::logger> client_logger;
        #endif
    };
}

#ifndef DIST_BUILD
// Core log macros
#define CORE_ERROR(...)       ::engine::Log::get_core_logger()->error(__VA_ARGS__)
#define CORE_WARN(...)        ::engine::Log::get_core_logger()->warn(__VA_ARGS__)
#define CORE_INFO(...)        ::engine::Log::get_core_logger()->info(__VA_ARGS__)
#define CORE_TRACE(...)       ::engine::Log::get_core_logger()->trace(__VA_ARGS__)
#define CORE_CRITICAL(...)    ::engine::Log::get_core_logger()->critical(__VA_ARGS__)

// Client log macros
#define CLIENT_ERROR(...)     ::engine::Log::get_client_logger()->error(__VA_ARGS__)
#define CLIENT_WARN(...)      ::engine::Log::get_client_logger()->warn(__VA_ARGS__)
#define CLIENT_INFO(...)      ::engine::Log::get_client_logger()->info(__VA_ARGS__)
#define CLIENT_TRACE(...)     ::engine::Log::get_client_logger()->trace(__VA_ARGS__)
#define CLIENT_CRITICAL(...)  ::engine::Log::get_client_logger()->critical(__VA_ARGS__)
#else
// Strip logs in Distribution/Release builds
#define CORE_ERROR(...)       ((void)0)
#define CORE_WARN(...)        ((void)0)
#define CORE_INFO(...)        ((void)0)
#define CORE_TRACE(...)       ((void)0)
#define CORE_CRITICAL(...)    ((void)0)

#define CLIENT_ERROR(...)     ((void)0)
#define CLIENT_WARN(...)      ((void)0)
#define CLIENT_INFO(...)      ((void)0)
#define CLIENT_TRACE(...)     ((void)0)
#define CLIENT_CRITICAL(...)  ((void)0)
#endif
