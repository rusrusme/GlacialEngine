//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "GameLayer.hpp"

#include "core/SystemContext.hpp"
#include "project/scene/SceneManager.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "project/scene/system/CameraSystem.hpp"
#include "render/RenderScope.hpp"

void GameLayer::on_attach(engine::SystemContext& context) {
    const auto scene = context.scenes.create("Main Scene");
    if (!scene) return;

    engine::Entity camera = scene->create_entity("Main Camera");
    camera.add_component<engine::CameraComponent>();
}

void GameLayer::on_detach(engine::SystemContext& context) {
}

void GameLayer::on_update(engine::SystemContext& context, float delta_time) {
    camera_system_.update(context.scenes.active_scene(), delta_time);
}

void GameLayer::on_render(engine::SystemContext& context, float delta_time) {
    const auto scene = context.scenes.active_scene();
    if (!scene) return;

    engine::Renderer::clear({0.12f, 0.12f, 0.12f, 1.0f});

    engine::RenderScope scope{context.renderer, delta_time};
    scope.begin();
    //
    scope.end();
}

void GameLayer::on_event(engine::SystemContext& context, engine::Event& event) {
}
