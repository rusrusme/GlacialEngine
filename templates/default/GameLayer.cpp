//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "GameLayer.hpp"

#include <GlacialEngine.hpp>

void GameLayer::on_attach(engine::SystemContext& context) {
    auto scene = context.scenes.create("Game");

    auto camera = scene->create_entity("Camera");
    camera.get_component<engine::TransformComponent>().position = {0.0f, 1.5f, 5.0f};
    camera.add_component<engine::CameraComponent>();
    camera.add_component<engine::FlyCameraControllerComponent>();

    context.scenes.on_play();
}

void GameLayer::on_detach(engine::SystemContext& context) {
}

void GameLayer::on_update(engine::SystemContext& context, float delta_time) {
    context.scenes.on_update(delta_time);
    if (auto active_scene = context.scenes.active_scene()) {
        engine::FlyCameraControllerSystem::on_update(*active_scene, delta_time);
    }
}

void GameLayer::on_render(engine::SystemContext& context, float delta_time) {
    if (auto active_scene = context.scenes.active_scene()) {
        context.renderer.render_scene(active_scene, context, delta_time);
    }
}

void GameLayer::on_event(engine::SystemContext& context, engine::Event& event) {
    context.scenes.on_event(event);
    if (auto active_scene = context.scenes.active_scene()) {
        engine::FlyCameraControllerSystem::on_event(*active_scene, event);
    }
}
