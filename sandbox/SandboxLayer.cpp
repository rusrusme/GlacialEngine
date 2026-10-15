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

    engine::AssetHandle firehouse_handle = context.assets.handle_from_path("assets/models/Firehouse.glb");

    if (!context.assets.asset_handle_valid(firehouse_handle)) {
        CORE_ERROR("Firehouse.glb konnte im AssetManager nicht gefunden werden!");
        return;
    }
    auto firehouse = scene->create_entity("Firehouse");
    firehouse.add_component<engine::MeshComponent>(firehouse_handle);

    auto& trans = firehouse.get_component<engine::TransformComponent>();
    trans.position = glm::vec3(0.0f, 0.0f, 0.0f);
    trans.scale = glm::vec3(1.0f);

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
        if (e.height() > 0) {
            // camera_.set_aspect_ratio(static_cast<float>(e.width()) / static_cast<float>(e.height()));
        }
        return false;
    });

    dispatcher.dispatch<engine::MouseScrolledEvent>([this](const engine::MouseScrolledEvent& e) {
        return true;
    });
}
