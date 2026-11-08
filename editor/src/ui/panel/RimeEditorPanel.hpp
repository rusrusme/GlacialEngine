//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include "ui/UIComponent.hpp"

namespace editor {
    using namespace engine;

    struct RimeEditorCallbackData {
        std::string* buffer = nullptr;
        int cursor_pos = 0;
    };

    class RimeEditorPanel : public UIComponent {
    public:
        RimeEditorPanel() : UIComponent("Rime") {}
        ~RimeEditorPanel() override = default;

        void on_render_ui(SystemContext& context) override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::Rime; }

    private:
        float scroll_x_ = 0.0f;
        float scroll_y_ = 0.0f;
        int cursor_pos_ = 0;
        int last_cursor_pos_ = -1;
        double cursor_blink_time_ = 0.0;
    };
}
