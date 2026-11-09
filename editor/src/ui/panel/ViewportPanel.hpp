//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/vec2.hpp>

#include "render/rhi/Framebuffer.hpp"
#include "ui/UIComponent.hpp"
#include "ui/overlay/GizmoOverlay.hpp"

namespace editor {
    using namespace engine;

    struct SceneViewport {
        Framebuffer* framebuffer = nullptr;
        glm::vec2 size{0.0f, 0.0f};
        glm::vec2 bounds_min{0.0f, 0.0f};
        glm::vec2 bounds_max{0.0f, 0.0f};
        bool hovered = false;
        bool focused = false;
        bool show_grid = true;
    };

    class ViewportPanel : public UIComponent {
    public:
        void on_update(SystemContext& context, float delta_time) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::Viewport; }

        [[nodiscard]] GizmoOverlay& gizmo() { return gizmo_; }

    private:
        GizmoOverlay gizmo_;
    };
}
