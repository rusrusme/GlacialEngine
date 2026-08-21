//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <RmlUi/Core/SystemInterface.h>

namespace engine {
    class RmlSystemInterface : public Rml::SystemInterface {
    public:
        double GetElapsedTime() override;
        bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
    };
}
