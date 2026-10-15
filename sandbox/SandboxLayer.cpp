//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include <random>

#include "core/Application.hpp"
#include "core/SystemContext.hpp"
#include "event/MouseEvents.hpp"
#include "input/Input.hpp"
#include "project/scene/SceneManager.hpp"
#include "project/scene/ecs/Entity.hpp"

void SandboxLayer::on_attach(engine::SystemContext& context) {
    const auto scene = context.scenes.create("Main Scene");
    scene->on_init();
}

void SandboxLayer::on_detach(engine::SystemContext& context) {
}

void SandboxLayer::on_update(engine::SystemContext& context, float delta_time) {
    camera_controller_.update(camera_, delta_time);
}

void SandboxLayer::on_render(engine::SystemContext& context, float delta_time) {
    if (const auto scene = context.scenes.active_scene()) {
        scene->on_render(context, delta_time);
    }
}

void SandboxLayer::on_event(engine::SystemContext& context, engine::Event& event) {
    engine::EventDispatcher dispatcher(event);
    dispatcher.dispatch<engine::WindowResizedEvent>([this](const engine::WindowResizedEvent& e) {
        return false;
    });

    dispatcher.dispatch<engine::MouseScrolledEvent>([this](const engine::MouseScrolledEvent& e) {
        return true;
    });
}
