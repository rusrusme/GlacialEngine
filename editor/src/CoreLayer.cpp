//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "CoreLayer.hpp"

#include "Config.hpp"
#include "core/SystemContext.hpp"
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

        if (Input::key_pressed_once(Key::SPACE)) {
            if (context.scenes.scene_state() == SceneState::Edit) {
                context.scenes.on_play();
            }
            else {
                context.scenes.on_stop();
            }
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

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizedEvent>([this](const WindowResizedEvent& e) {
            if (e.height() > 0) {
                cam_.set_aspect_ratio(static_cast<float>(e.width()) / static_cast<float>(e.height()));
            }
            return false;
        });
    }
}
