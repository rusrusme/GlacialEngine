//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/vec2.hpp>

#include "render/rhi/Framebuffer.hpp"
#include "ui/UIComponent.hpp"

namespace editor {
    using namespace engine;

    struct SceneViewport {
        Framebuffer* framebuffer = nullptr;
        glm::vec2 size{0.0f, 0.0f};
        bool hovered = false;
        bool focused = false;
    };

    class ViewportPanel : public UIComponent {
    public:
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::Viewport; }
    };
}
