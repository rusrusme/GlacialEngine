//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    class HistoryPanel : public UIComponent {
    public:
        HistoryPanel() : UIComponent("History") {}
        ~HistoryPanel() override = default;

        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::History; }

        void toggle_visible() { visible_ = !visible_; }
        [[nodiscard]] bool visible() const noexcept { return visible_; }
        void set_visible(bool b) noexcept { visible_ = b; }

    private:
        bool visible_ = true;
    };
}
