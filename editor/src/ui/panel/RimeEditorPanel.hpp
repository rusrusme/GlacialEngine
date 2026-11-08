//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "ui/UIComponent.hpp"

namespace editor {
    using namespace engine;

    class RimeEditorPanel : public UIComponent {
    public:
        RimeEditorPanel() : UIComponent("Rime") {}
        ~RimeEditorPanel() override = default;

        void on_render_ui(SystemContext& context) override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::Rime; }
    };
}
