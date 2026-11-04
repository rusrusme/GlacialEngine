//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    using namespace engine;

    class DebugPanel : public UIComponent {
    public:
        DebugPanel() = default;
        ~DebugPanel() override = default;

        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;
    };
}
