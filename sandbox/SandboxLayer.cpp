//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include "core/Application.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "render/rhi/RenderContext.hpp"
#include "script/ScriptEngine.hpp"

// TODO Add ScriptComponent in Component.hpp

void SandboxLayer::on_attach(SystemContext& context) {
    #if defined(_WIN32)
    ScriptEngine::load_library("assets/scripts/Scripts.dll");
    #else
    ScriptEngine::load_library("assets/scripts/Scripts.so");
    #endif

    camera_.set_far_clip(10);
    camera_.set_near_clip(0.1f);
    camera_.set_fov(60);
    camera_.set_distance(4);

    const auto& swapchain = context.render_context.swapchain();
    camera_.set_viewport_size(swapchain.width(), swapchain.height());
}

void SandboxLayer::on_detach(SystemContext& context) {
}

void SandboxLayer::on_update(SystemContext& context, float delta_time) {
    auto scene = context.scenes.active_scene();
    if (!scene) return;

    context.scenes.on_update(delta_time);
    camera_controller_.on_update(camera_);

    if (Input::key_pressed_once(Key::SPACE)) {
        if (context.scenes.scene_state() == SceneState::Edit) {
            context.scenes.on_play();
        }
        else {
            context.scenes.on_stop();
        }
    }

    if (Input::key_pressed_once(Key::F5)) {
        ScriptEngine::request_reload();
    }
    ScriptEngine::on_update(context.scenes.active_scene().get());
}

void SandboxLayer::on_render(SystemContext& context, float delta_time) {
    const auto& swapchain = context.render_context.swapchain();
    if (swapchain.width() == 0 || swapchain.height() == 0) return;

    auto active_scene = context.scenes.active_scene();
    if (!active_scene) return;

    camera_.set_viewport_size(swapchain.width(), swapchain.height());

    context.renderer.render_scene(active_scene, context, delta_time, &camera_);
}

void SandboxLayer::on_event(SystemContext& context, Event& event) {
    camera_controller_.on_event(camera_, event);
    if (!event.handled) {
        context.scenes.on_event(event);
    }
}
