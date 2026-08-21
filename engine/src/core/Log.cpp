//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/Log.hpp"

namespace engine {
    void Log::init() {
        #ifdef ENABLE_SPDLOG
        spdlog::set_pattern("%^[%T] %n: %v%$");
        core_logger = spdlog::stdout_color_mt("ENGINE");
        core_logger->set_level(spdlog::level::trace);

        client_logger = spdlog::stdout_color_mt("APP");
        client_logger->set_level(spdlog::level::trace);
        #endif
    }

    #ifdef ENABLE_SPDLOG
    std::shared_ptr<spdlog::logger> Log::core_logger;
    std::shared_ptr<spdlog::logger> Log::client_logger;
    #endif
}
