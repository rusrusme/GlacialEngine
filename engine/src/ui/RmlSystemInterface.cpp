//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ui/RmlSystemInterface.hpp"

#include "core/Log.hpp"
#include "core/WindowAPI.hpp"

namespace engine {
    double RmlSystemInterface::GetElapsedTime() {
        return window::get_time();
    }

    bool RmlSystemInterface::LogMessage(const Rml::Log::Type type, const Rml::String& message) {
        switch (type) {
            case Rml::Log::LT_ERROR :
                CORE_ERROR("[RmlUi] {0}", message);
                break;
            case Rml::Log::LT_WARNING :
                CORE_WARN("[RmlUi] {0}", message);
                break;
            default :
                CORE_TRACE("[RmlUi] {0}", message);
                break;
        }
        return true;
    }
}
