//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    class Menubar : public UIComponent {
    public:
        Menubar() = default;
        ~Menubar() override = default;

        void on_init(SystemContext& context) override;
        void on_render_ui(SystemContext& context) override;
    };
}
