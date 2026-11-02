//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    using namespace engine;

    class Console : UIComponent {
    public:
        Console() = default;
        ~Console() override = default;

        void on_render_ui(SystemContext& context) override;
    };
}
