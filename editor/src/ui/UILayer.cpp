//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "UILayer.hpp"

#include "EditorSettings.hpp"
#include "core/Application.hpp"
#include "event/EditorActions.hpp"
#include "event/EditorEvents.hpp"
#include "event/MouseEvents.hpp"
#include "input/Input.hpp"
#include "rime/RimeEditor.hpp"
#include "script/ScriptEngine.hpp"

namespace editor {
    void UILayer::on_attach(SystemContext& context) {
        ImguiLayer::on_attach(context);

        ImGuiIO& io = ImGui::GetIO();
        ImFontConfig config;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.FontGlobalScale = 1.0f;
        const float dpiScale = Application::get().window().monitor_dpi_scale();

        s_code_font = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/JetBrainsMono-VariableFont.ttf",
            16.0f * dpiScale,
            &config
        );

        ImGuizmo::Style& style = ImGuizmo::GetStyle();
        style.TranslationLineThickness = 3.0f;
        style.TranslationLineArrowSize = 5.0f;
        style.CenterCircleSize = 4.0f;
        style.HatchedAxisLineThickness = 0.0f;

        ui_components_ = {
            &layout_,
            &menubar_,
            &toolbar_,
            &settings_window_,
            &inspector_panel_,
            &hierarchy_panel_,
            &asset_browser_panel_,
            &console_panel_,
            &debug_panel_,
            &viewport_panel_,
            &history_panel_,
            &rime_panel
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
        for (auto* component : ui_components_) {
            component->on_update(context, delta_time);
        }
    }

    void UILayer::on_event(SystemContext& context, Event& event) {
        ImguiLayer::on_event(context, event);

        const auto& viewport = context.get<SceneViewport>();
        if (viewport.hovered) {
            if (event.get_event_type() == engine::EventType::MouseScrolled ||
                event.get_event_type() == engine::EventType::MouseButtonPressed ||
                event.get_event_type() == engine::EventType::MouseButtonReleased) {
                event.handled = false;
            }
        }

        if (event.handled) return;

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseScrolledEvent>([&context](const MouseScrolledEvent& e) {
            auto& rime_settings = context.get<EditorSettings>().get<RimeSettings>();
            bool ctrl = Input::key_pressed(Key::LEFT_CONTROL) || Input::key_pressed(Key::RIGHT_CONTROL);

            if (Shortcuts::active_context() == ShortcutContext::Rime && ctrl) {
                if (!rime_settings.change_size_on_scroll) return false;

                rime_settings.scale_font(e.y_offset());
                return true;
            }
            return false;
        });

        dispatcher.dispatch<EditorActionEvent>([this, &context](const EditorActionEvent& action) {
            switch (action.action()) {
                case Action::SaveRime : {
                    auto& rime = context.get<RimeContext>();
                    auto* active_doc = rime.active_document();
                    std::ofstream out(active_doc->filepath);
                    out << active_doc->buffer;
                    active_doc->dirty = false;

                    if (auto scene = context.scenes.active_scene()) {
                        ScriptEngine::reload(scene.get());
                    }
                    else {
                        CORE_ERROR("Failed to reload scripts: No valid scene found");
                    }

                    return true;
                }

                case Action::ToggleDebug :
                    debug_panel_.toggle_visible();
                    return true;
                case Action::ToggleConsole :
                    console_panel_.toggle_visible();
                    return true;
                case Action::ToggleInspector :
                    inspector_panel_.toggle_visible();
                    return true;
                case Action::ToggleHierarchy :
                    hierarchy_panel_.toggle_visible();
                    return true;
                case Action::ToggleViewport :
                    viewport_panel_.toggle_visible();
                    return true;
                case Action::ToggleAssetBrowser :
                    asset_browser_panel_.toggle_visible();
                    return true;
                case Action::ToggleHistory :
                    history_panel_.toggle_visible();
                    return true;
                case Action::ToggleSettings :
                    settings_window_.toggle_visible();
                    return true;
                case Action::ToggleRime :
                    rime_panel.toggle_visible();
                    return true;

                case Action::CreateEntity : {
                    hierarchy_panel_.create_entity();
                    return true;
                }
                case Action::DeleteEntity : {
                    hierarchy_panel_.delete_entity();
                    return true;
                }
                case Action::RenameEntity : {
                    hierarchy_panel_.rename_entity();
                    return true;
                }
                case Action::DuplicateEntity : {
                    hierarchy_panel_.duplicate_entity();
                    return true;
                }
                case Action::DeselectEntity :
                    hierarchy_panel_.deselect_entity();
                    return true;

                case Action::Exit :
                    Application::get().close();
                    return true;
                default : break;
            }
            return false;
        });
    }

    void UILayer::on_imgui_render(SystemContext& context, float delta_time) {
        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
            Shortcuts::set_active_context(ShortcutContext::Global);
        }

        debug_panel_.on_render_ui(context);
        settings_window_.on_render_ui(context);

        layout_.on_render_ui(context);
        menubar_.on_render_ui(context);
        toolbar_.on_render_ui(context);
        asset_browser_panel_.on_render_ui(context);
        console_panel_.on_render_ui(context);
        inspector_panel_.on_render_ui(context);
        hierarchy_panel_.on_render_ui(context);
        viewport_panel_.on_render_ui(context);
        history_panel_.on_render_ui(context);
        rime_panel.on_render_ui(context);
    }
}
