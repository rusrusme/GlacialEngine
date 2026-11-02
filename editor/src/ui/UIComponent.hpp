//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/SystemContext.hpp"

namespace editor {
    using namespace engine;

    class UIComponent {
    public:
        UIComponent() = default;
        virtual ~UIComponent() = default;

        virtual void on_init(SystemContext& context) {}
        virtual void on_render_ui(SystemContext& context) = 0;

        virtual void load_config() {}
        virtual void save_config() {}
    };
}
