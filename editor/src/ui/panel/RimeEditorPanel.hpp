//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

#include "ui/UIComponent.hpp"

namespace editor {
    using namespace engine;

    struct TextDocument;

    struct RimeEditorCallbackData {
        std::string* buffer = nullptr;
        int* cursor_pos = nullptr;
    };

    class RimeEditorPanel : public UIComponent {
    public:
        RimeEditorPanel() : UIComponent("Rime") {}
        ~RimeEditorPanel() override = default;

        void on_render_ui(SystemContext& context) override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::Rime; }

    private:
        static void render_empty_state();
        static void render_editor_view(SystemContext& context, TextDocument& doc, const ImVec2& avail);
        static void render_status_bar(SystemContext& context, const TextDocument& doc, const ImVec2& start_pos, float width, float bar_height, float editor_height);
    };
}
