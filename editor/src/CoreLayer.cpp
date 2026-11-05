//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "CoreLayer.hpp"

#include "Config.hpp"
#include "core/Application.hpp"
#include "core/SystemContext.hpp"
#include "event/EditorEvents.hpp"
#include "event/EventQueue.hpp"
#include "event/KeyEvents.hpp"
#include "event/ShortcutManager.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "input/MouseCodes.hpp"
#include "project/scene/SceneManager.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "render/SceneRenderer.hpp"
#include "render/rhi/RenderContext.hpp"
#include "script/ScriptEngine.hpp"
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
        context.scenes.active_scene()->create_cube("A CUBE");
        config::load();

        const auto& swapchain = context.render_context.swapchain();
        cam_.set_viewport_size(swapchain.width(), swapchain.height());

        // TODO Add support for JSON binding: {"bindings": [{"action": "SaveScene", "key": "Ctrl+S", "context": "global"}, ...]}
        /////////////////////////////////////////////////////////////////////////////
        // Keybindings
        ShortcutManager::bind(EditorAction::NewScene, "Ctrl+N", Key::N, Ctrl);
        ShortcutManager::bind(EditorAction::OpenScene, "Ctrl+O", Key::O, Ctrl);
        ShortcutManager::bind(EditorAction::SaveScene, "Ctrl+S", Key::S, Ctrl);
        ShortcutManager::bind(EditorAction::SaveSceneAs,
                              "Ctrl+Shift+S",
                              Key::S, Ctrl | Shift);

        ShortcutManager::bind(EditorAction::Undo, "Ctrl+Z", Key::Z, Ctrl);
        ShortcutManager::bind(EditorAction::Redo, "Ctrl+Y", Key::Y, Ctrl);
        ShortcutManager::bind(EditorAction::Copy, "Ctrl+C", Key::C, Ctrl);
        ShortcutManager::bind(EditorAction::Paste, "Ctrl+V", Key::V, Ctrl);

        ShortcutManager::bind(EditorAction::DuplicateEntity, "Ctrl+D",
                              Key::D, Ctrl, ShortcutContext::Hierarchy);

        ShortcutManager::bind(EditorAction::DeleteEntity, "Delete",
                              Key::DELETE, None, ShortcutContext::Hierarchy);

        ShortcutManager::bind(EditorAction::RenameEntity, "F2",
                              Key::F2, None, ShortcutContext::Hierarchy);

        ShortcutManager::bind(EditorAction::DeselectEntity, "Escape",
                              Key::ESCAPE, None, ShortcutContext::Hierarchy);

        ShortcutManager::bind(EditorAction::TogglePlay, "Space", Key::SPACE);
        ShortcutManager::bind(EditorAction::StopPlay, "Shift+Space", Key::SPACE, Shift);

        ShortcutManager::bind(EditorAction::GizmoSelect, "Q", Key::Q, None, ShortcutContext::Viewport);
        ShortcutManager::bind(EditorAction::GizmoTranslate, "W", Key::W, None, ShortcutContext::Viewport);
        ShortcutManager::bind(EditorAction::GizmoRotate, "E", Key::E, None, ShortcutContext::Viewport);
        ShortcutManager::bind(EditorAction::GizmoScale, "R", Key::R, None, ShortcutContext::Viewport);

        ShortcutManager::bind(EditorAction::ToggleConsole,
                              "Ctrl+Shift+`",
                              Key::GRAVE_ACCENT, Ctrl | Shift);
        ShortcutManager::bind(EditorAction::ToggleConsole, "Alt+F12", Key::F12, Alt);
        ShortcutManager::bind(EditorAction::OpenSettings, "Ctrl+,", Key::COMMA, Ctrl);
        ShortcutManager::bind(EditorAction::ToggleDebug, "F3", Key::F3);
    }

    void CoreLayer::on_detach(SystemContext& context) {
        config::save();
    }

    void CoreLayer::on_update(SystemContext& context, float delta_time) {
        PROFILE_FUNCTION();

        const bool can_control_cam = viewport_hovered_ || viewport_focused_
            || Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_MIDDLE);

        if (can_control_cam) {
            cam_controller_.on_update(cam_);
        }

        auto scene = context.scenes.active_scene();
        if (!scene) return;

        context.scenes.on_update(delta_time);
        cam_controller_.on_update(cam_);
    }

    void CoreLayer::on_render(SystemContext& context, float delta_time) {
        PROFILE_FUNCTION();

        const auto& swapchain = context.render_context.swapchain();
        if (swapchain.width() == 0 || swapchain.height() == 0) return;

        auto active_scene = context.scenes.active_scene();
        if (!active_scene) return;

        cam_.set_viewport_size(swapchain.width(), swapchain.height());

        context.renderer.render_scene(active_scene, context, delta_time, &cam_);
    }

    void CoreLayer::on_event(SystemContext& context, Event& event) {
        PROFILE_FUNCTION();

        cam_controller_.on_event(cam_, event);
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

        dispatcher.dispatch<KeyPressedEvent>([this](const KeyPressedEvent& e) {
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

                ShortcutContext current_context = viewport_hovered_ || viewport_focused_
                                                    ? ShortcutContext::Viewport
                                                    : ShortcutManager::active_context();

                auto action = ShortcutManager::find_action(e.key_code(), active_modifier, current_context);
                if (action.has_value()) {
                    EventQueue::submit<EditorActionEvent>(action.value());
                    return true;
                }
                return false;
            }
        );

        dispatcher.dispatch<EditorActionEvent>([&context](const EditorActionEvent& action) {
            switch (action.action()) {
                case EditorAction::NewScene :
                    break;
                case EditorAction::OpenScene :
                    break;
                case EditorAction::SaveScene :
                    break;
                case EditorAction::SaveSceneAs :
                    break;
                case EditorAction::NewProject :
                    break;
                case EditorAction::OpenProject :
                    break;
                case EditorAction::SaveProject :
                    break;
                case EditorAction::SaveProjectAs :
                    break;
                case EditorAction::Exit :
                    break;
                case EditorAction::Undo :
                    break;
                case EditorAction::Redo :
                    break;
                case EditorAction::Copy :
                    break;
                case EditorAction::Paste :
                    break;
                case EditorAction::DuplicateEntity :
                    break;
                case EditorAction::DeleteEntity :
                    break;
                case EditorAction::TogglePlay :
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
                case EditorAction::StopPlay :
                    if (context.scenes.scene_state() != SceneState::Edit) {
                        context.scenes.on_stop();
                    }
                    return true;
                case EditorAction::GizmoSelect :
                    break;
                case EditorAction::GizmoTranslate :
                    break;
                case EditorAction::GizmoRotate :
                    break;
                case EditorAction::GizmoScale :
                    break;
                default : break;
            }
            return false;
        });
    }
}
