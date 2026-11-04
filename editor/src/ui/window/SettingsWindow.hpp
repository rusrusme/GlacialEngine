//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    class SettingsWindow : public UIComponent {
    public:
        SettingsWindow() = default;
        ~SettingsWindow() override = default;

        void on_init(SystemContext& context) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;
    };
}
