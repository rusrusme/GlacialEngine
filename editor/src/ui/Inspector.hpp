//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    class Inspector : public UIComponent {
    public:
        Inspector() = default;
        ~Inspector() override = default;

        void on_render_ui(SystemContext& context) override;
    };
}
