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

        void toggle_visible() { visible_ = !visible_; }
        [[nodiscard]] bool visible() const noexcept { return visible_; }
        void set_visible(bool b) noexcept { visible_ = b; }

    private:
        bool visible_ = true;
    };
}
