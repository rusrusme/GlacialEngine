//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "UILayer.hpp"

#include "event/EditorActions.hpp"
#include "event/EditorEvents.hpp"

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
            &debug_panel_,
            &viewport_panel_
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
        debug_panel_.on_update(context, delta_time);
        hierarchy_.on_update(context, delta_time);
    }

    void UILayer::on_event(SystemContext& context, Event& event) {
        ImguiLayer::on_event(context, event);

        if (event.handled) return;

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<EditorActionEvent>([this](const EditorActionEvent& action) {
            switch (action.action()) {
                case EditorAction::ToggleDebug :
                    debug_panel_.toggle_visible();
                    return true;
                case EditorAction::ToggleConsole :
                    console_panel_.toggle_visible();
                    return true;
                case EditorAction::ToggleInspector :
                    inspector_.toggle_visible();
                    return true;
                case EditorAction::ToggleHierarchy :
                    hierarchy_.toggle_visible();
                    return true;
                case EditorAction::OpenSettings :
                    settings_.toggle_visible();
                    return true;
                case EditorAction::DeleteEntity :
                    hierarchy_.delete_entity();
                    return true;
                case EditorAction::RenameEntity :
                    hierarchy_.rename_entity();
                    return true;
                case EditorAction::DeselectEntity :
                    hierarchy_.deselect_entity();
                    return true;
                case EditorAction::DuplicateEntity :
                    hierarchy_.duplicate_entity();
                    return true;
                default : break;
            }
            return false;
        });
    }

    void UILayer::on_imgui_render(SystemContext& context, float delta_time) {
        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
            ShortcutManager::set_active_context(ShortcutContext::Global);
        }

        debug_panel_.on_render_ui(context);
        settings_.on_render_ui(context);

        layout_.on_render_ui(context);
        menubar_.on_render_ui(context);
        toolbar_.on_render_ui(context);
        asset_browser_.on_render_ui(context);
        console_panel_.on_render_ui(context);
        inspector_.on_render_ui(context);
        hierarchy_.on_render_ui(context);
        viewport_panel_.on_render_ui(context);
    }
}
