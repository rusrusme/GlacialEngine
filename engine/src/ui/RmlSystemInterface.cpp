//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ui/RmlSystemInterface.hpp"

namespace engine {
    double RmlSystemInterface::GetElapsedTime() {
        return glfwGetTime();
    }

    bool RmlSystemInterface::LogMessage(const Rml::Log::Type type, const Rml::String& message) {
        switch (type) {
            case Rml::Log::LT_ERROR :
                std::cerr << "[RmlUi Error] " << message << std::endl;
                break;
            case Rml::Log::LT_WARNING :
                std::cerr << "[RmlUi Warning] " << message << std::endl;
                break;
            default :
                std::cout << "[RmlUi] " << message << std::endl;
                break;
        }
        return true;
    }
}
