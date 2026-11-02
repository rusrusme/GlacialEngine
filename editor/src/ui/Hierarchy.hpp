//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    class Hierarchy : public UIComponent {
    public:
        Hierarchy() = default;
        ~Hierarchy() override;

        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;
    };
}
