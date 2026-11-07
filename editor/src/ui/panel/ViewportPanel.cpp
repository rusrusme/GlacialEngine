//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ViewportPanel.hpp"

#include "Config.hpp"
#include "core/SystemContext.hpp"

namespace editor {
    void ViewportPanel::on_render_ui(SystemContext& context) {
        auto& viewport = context.get<SceneViewport>();

        if (!visible_) {
            viewport.hovered = false;
            viewport.focused = false;
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
        ImGui::Begin("Scene");

        viewport.focused = ImGui::IsWindowFocused();
        if (viewport.focused) {
            Shortcuts::set_active_context(shortcut_context());
        }

        const ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x > 0.0f && avail.y > 0.0f) {
            viewport.size = {avail.x, avail.y};
        }

        if (viewport.framebuffer && viewport.framebuffer->color_attachment_count() > 0) {
            const uint32_t texture_id = viewport.framebuffer->color_attachment_renderer_id(0);

            // TODO Make a distinction for Vulkan, OpenGL and D3D12
            //      OpenGL texture origin is bottom-left; flip UV coordinates vertically for ImGui
            ImGui::Image(
                texture_id,
                avail,
                ImVec2(0.0f, 1.0f),
                ImVec2(1.0f, 0.0f)
            );
        }
        viewport.hovered = ImGui::IsItemHovered();

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ViewportPanel::load_config() {
        visible_ = config::get_value("ViewportPanel_visible", visible_);
    }

    void ViewportPanel::save_config() {
        config::set_value("ViewportPanel_visible", visible_);
    }
}
