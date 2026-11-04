//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "UILayer.hpp"

namespace editor {
    void UILayer::on_attach(SystemContext& context) {
        ImguiLayer::on_attach(context);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.FontGlobalScale = 1.0f;

        ImGuizmo::Style& style = ImGuizmo::GetStyle();
        style.TranslationLineThickness = 3.0f;
        style.TranslationLineArrowSize = 5.0f;
        style.CenterCircleSize = 4.0f;
        style.HatchedAxisLineThickness = 0.0f;

        ui_components_ = {
            &layout_,
            &menubar_,
            &toolbar_,
            &settings_,
            &inspector_,
            &hierarchy_,
            &asset_browser_,
            &console_panel_,
            &debug_panel_
        };

        for (auto* component : ui_components_) {
            component->on_init(context);
            component->load_config();
        }
    }

    void UILayer::on_detach(SystemContext& context) {
        for (auto* component : ui_components_) {
            component->save_config();
        }

        ImguiLayer::on_detach(context);
    }

    void UILayer::on_update(SystemContext& context, float delta_time) {
        ImguiLayer::on_update(context, delta_time);
    }

    void UILayer::on_event(SystemContext& context, Event& event) {
        ImguiLayer::on_event(context, event);
    }

    void UILayer::on_imgui_render(SystemContext& context, float delta_time) {
        console_panel_.on_render_ui(context);
    }
}
