//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "CoreLayer.hpp"

#include "Config.hpp"
#include "EditorSettings.hpp"
#include "command/CommandHistory.hpp"
#include "command/Transaction.hpp"
#include "core/Application.hpp"
#include "core/SystemContext.hpp"
#include "event/EditorEvents.hpp"
#include "event/EventQueue.hpp"
#include "event/KeyEvents.hpp"
#include "event/Shortcuts.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "input/MouseCodes.hpp"
#include "project/scene/SceneManager.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "render/SceneRenderer.hpp"
#include "render/rhi/RenderContext.hpp"
#include "script/ScriptEngine.hpp"
#include "ui/panel/ViewportPanel.hpp"
#include "util/Instrumentor.hpp"

namespace editor {
    void CoreLayer::on_attach(SystemContext& context) {
        PROFILE_FUNCTION();

        #if defined(_WIN32)
        ScriptEngine::load_library("assets/scripts/Scripts.dll");
        #else
        ScriptEngine::load_library("assets/scripts/Scripts.so");
        #endif

        context.scenes.create();
        config::load();
        context.get<EditorSettings>().load_all();

        FramebufferSpecification fb_spec{};
        fb_spec.width = 1280;
        fb_spec.height = 720;
        fb_spec.swapchain_target = false;
        fb_spec.attachments = {
            TextureFormat::RGBA8,
            TextureFormat::Depth24Stencil8
        };
        framebuffer_ = context.render_context.device().create_framebuffer(fb_spec);

        auto& viewport = context.get<SceneViewport>();
        viewport.framebuffer = framebuffer_.get();
        viewport.size = {1280.0f, 720.0f};

        cam_.set_viewport_size(1280, 720);

        // TODO Add support for JSON binding: {"bindings": [{"action": "SaveScene", "key": "Ctrl+S", "context": "global"}, ...]}
        /////////////////////////////////////////////////////////////////////////////
        // Keybindings
        Shortcuts::bind(Action::NewScene, "Ctrl+N", Key::N, Ctrl);
        Shortcuts::bind(Action::OpenScene, "Ctrl+O", Key::O, Ctrl);
        Shortcuts::bind(Action::SaveScene, "Ctrl+S", Key::S, Ctrl);
        Shortcuts::bind(Action::SaveSceneAs,
                        "Ctrl+Shift+S",
                        Key::S, Ctrl | Shift);

        Shortcuts::bind(Action::Undo, "Ctrl+Z", Key::Z, Ctrl);
        Shortcuts::bind(Action::Redo, "Ctrl+Y", Key::Y, Ctrl);
        Shortcuts::bind(Action::Copy, "Ctrl+C", Key::C, Ctrl);
        Shortcuts::bind(Action::Paste, "Ctrl+V", Key::V, Ctrl);

        Shortcuts::bind(Action::DuplicateEntity, "Ctrl+D",
                        Key::D, Ctrl, ShortcutContext::Hierarchy);

        Shortcuts::bind(Action::DeleteEntity, "Delete",
                        Key::DELETE, None, ShortcutContext::Hierarchy);

        Shortcuts::bind(Action::RenameEntity, "F2",
                        Key::F2, None, ShortcutContext::Hierarchy);

        Shortcuts::bind(Action::DeselectEntity, "Escape",
                        Key::ESCAPE, None, ShortcutContext::Hierarchy);

        Shortcuts::bind(Action::TogglePlay, "Space", Key::SPACE);
        Shortcuts::bind(Action::StopPlay, "Shift+Space", Key::SPACE, Shift);

        Shortcuts::bind(Action::GizmoSelect, "Q", Key::Q, None, ShortcutContext::Viewport);
        Shortcuts::bind(Action::GizmoTranslate, "W", Key::W, None, ShortcutContext::Viewport);
        Shortcuts::bind(Action::GizmoRotate, "E", Key::E, None, ShortcutContext::Viewport);
        Shortcuts::bind(Action::GizmoScale, "R", Key::R, None, ShortcutContext::Viewport);

        Shortcuts::bind(Action::ToggleHierarchy, "Alt+1", Key::NUM_1, Alt);
        Shortcuts::bind(Action::ToggleInspector, "Alt+2", Key::NUM_2, Alt);
        Shortcuts::bind(Action::ToggleViewport, "Alt+3", Key::NUM_3, Alt);
        Shortcuts::bind(Action::ToggleAssetBrowser, "Alt+4", Key::NUM_4, Alt);
        Shortcuts::bind(Action::ToggleRime, "Alt+5", Key::NUM_5, Alt);
        Shortcuts::bind(Action::ToggleConsole, "Alt+6", Key::NUM_6, Alt);
        Shortcuts::bind(Action::ToggleHistory, "Alt+7", Key::NUM_7, Alt);
        Shortcuts::bind(Action::ToggleConsole, // Alternative binding
                        "Ctrl+Shift+`",
                        Key::GRAVE_ACCENT, Ctrl | Shift);
        Shortcuts::bind(Action::ToggleSettings, "Ctrl+,", Key::COMMA, Ctrl);
        Shortcuts::bind(Action::ToggleDebug, "F3", Key::F3);

        Shortcuts::bind(Action::SaveRime, "Ctrl+S", Key::S, Ctrl, ShortcutContext::Rime);
    }

    void CoreLayer::on_detach(SystemContext& context) {
        context.get<EditorSettings>().save_all();
        config::save();
        framebuffer_.reset();
    }

    void CoreLayer::on_update(SystemContext& context, float delta_time) {
        PROFILE_FUNCTION();

        const auto& viewport = context.get<SceneViewport>();

        const bool rmb = Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_RIGHT);
        const bool mmb = Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_MIDDLE);
        const bool is_nav_button_down = rmb || mmb;

        // Continue active drag until button is released
        if (controlling_cam_) {
            if (!is_nav_button_down) {
                controlling_cam_ = false;
                cam_controller_.reset();
            }
        }
        // Only start a drag if the click began inside the viewport
        else {
            if (is_nav_button_down && viewport.hovered) {
                controlling_cam_ = true;
                cam_controller_.reset();
            }
        }

        if (controlling_cam_) {
            cam_controller_.on_update(cam_);
        }
        else {
            cam_controller_.reset();
        }

        auto scene = context.scenes.active_scene();
        if (!scene) return;

        context.scenes.on_update(delta_time);
    }

    void CoreLayer::on_render(SystemContext& context, float delta_time) {
        PROFILE_FUNCTION();

        auto active_scene = context.scenes.active_scene();
        if (!active_scene || !framebuffer_) return;

        auto& viewport = context.get<SceneViewport>();

        if (viewport.size.x > 0.0f && viewport.size.y > 0.0f) {
            const auto& spec = framebuffer_->specification();
            const auto target_w = static_cast<uint32_t>(viewport.size.x);
            const auto target_h = static_cast<uint32_t>(viewport.size.y);

            if (spec.width != target_w || spec.height != target_h) {
                framebuffer_->resize(target_w, target_h);
                cam_.set_viewport_size(target_w, target_h);
            }
        }

        context.renderer.render_scene(active_scene, context, delta_time, &cam_, framebuffer_.get());
    }

    void CoreLayer::on_event(SystemContext& context, Event& event) {
        PROFILE_FUNCTION();

        const auto& viewport = context.get<SceneViewport>();
        if (event.get_event_type() == engine::EventType::MouseScrolled) {
            if (viewport.hovered) {
                cam_controller_.on_event(cam_, event);
            }
            event.handled = true;
            return;
        }

        if (!event.handled) {
            context.scenes.on_event(event);
        }

        if (event.handled) return;

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizedEvent>([this](const WindowResizedEvent& e) {
            if (e.height() > 0) {
                cam_.set_aspect_ratio(static_cast<float>(e.width()) / static_cast<float>(e.height()));
            }
            return false;
        });

        dispatcher.dispatch<KeyPressedEvent>([&context](const KeyPressedEvent& e) {
            KeyModifier active_modifier = None;
            if (Input::key_pressed(Key::LEFT_CONTROL) || Input::key_pressed(Key::RIGHT_CONTROL)) {
                active_modifier |= Ctrl;
            }
            if (Input::key_pressed(Key::LEFT_SHIFT) || Input::key_pressed(Key::RIGHT_SHIFT)) {
                active_modifier |= Shift;
            }
            if (Input::key_pressed(Key::LEFT_ALT) || Input::key_pressed(Key::RIGHT_ALT)) {
                active_modifier |= Alt;
            }

            const auto& viewport = context.get<SceneViewport>();
            ShortcutContext current_context = (viewport.hovered || viewport.focused)
                                                  ? ShortcutContext::Viewport
                                                  : Shortcuts::active_context();

            auto action = Shortcuts::find_action(e.key_code(), active_modifier, current_context);
            if (action.has_value()) {
                EventQueue::submit<EditorActionEvent>(action.value());
                return true;
            }
            return false;
        });

        dispatcher.dispatch<EditorActionEvent>([&context](const EditorActionEvent& action) {
            auto& history = context.get<CommandHistory>();

            switch (action.action()) {
                case Action::NewScene :
                    return true;
                case Action::OpenScene :
                    return true;
                case Action::SaveScene :
                    return true;
                case Action::SaveSceneAs :
                    return true;
                case Action::NewProject :
                    return true;
                case Action::OpenProject :
                    return true;
                case Action::SaveProject :
                    return true;
                case Action::SaveProjectAs :
                    return true;

                case Action::Copy : {
                    Transaction transaction("Copy Selection", history);
                    // TODO Copy logic
                    return true;
                }
                case Action::Paste : {
                    Transaction transaction("Paste Selection", history);
                    // TODO Paste logic
                    return true;
                }

                case Action::Undo :
                    if (history.can_undo()) {
                        history.undo(context);
                    }
                    return true;
                case Action::Redo :
                    if (history.can_redo()) {
                        history.redo(context);
                    }
                    return true;

                case Action::TogglePlay :
                    if (context.scenes.scene_state() == SceneState::Edit) {
                        context.scenes.on_play();
                    }
                    else if (context.scenes.scene_state() == SceneState::Play) {
                        context.scenes.on_pause();
                    }
                    else if (context.scenes.scene_state() == SceneState::Pause) {
                        context.scenes.on_play();
                    }
                    return true;
                case Action::StopPlay :
                    if (context.scenes.scene_state() != SceneState::Edit) {
                        context.scenes.on_stop();
                    }
                    return true;
                default : break;
            }
            return false;
        });
    }
}
