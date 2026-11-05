//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    class ViewportPanel : public UIComponent {
    public:
        void on_render_ui(SystemContext& context) override;
    };
}
