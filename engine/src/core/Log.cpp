//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/Log.hpp"

#ifndef DIST_BUILD
#include "spdlog/sinks/stdout_color_sinks.h"

namespace engine {
    void Log::init() {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        core_logger = spdlog::stdout_color_mt("ENGINE");
        core_logger->set_level(spdlog::level::trace);

        client_logger = spdlog::stdout_color_mt("APP");
        client_logger->set_level(spdlog::level::trace);
    }

    std::shared_ptr<spdlog::logger> Log::core_logger;
    std::shared_ptr<spdlog::logger> Log::client_logger;
}
#else
namespace engine {
    void Log::init() {
        // No-op in distribution build
    }
}
#endif
