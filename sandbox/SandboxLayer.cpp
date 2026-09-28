//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include "core/Application.hpp"
#include "core/SystemContext.hpp"
#include "event/MouseEvents.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "render/debug/DebugRenderer.hpp"
#include "scene/SceneManager.hpp"
#include "scene/ecs/Entity.hpp"

void SandboxLayer::on_attach(engine::SystemContext& context) {
    const auto scene = context.scenes.create("Main Scene");

    camera_.init();

    scene->create_cube("John",
                       glm::vec4{1.0f, 0.5f, 0.2f, 1.0f}
    );
    scene->create_point_light("Light",
                              {10.0f, 10.0f, 10.0f},
                              {1.0f, 0.95f, 0.8f},
                              1.2f,
                              25.0f
    );
}

void SandboxLayer::on_detach(engine::SystemContext& context) {
}

void SandboxLayer::on_update(engine::SystemContext& context, float delta_time) {
    camera_controller_.update(camera_, delta_time);
    auto scene = context.scenes.active_scene();
    if (!scene) return;
    if (engine::Input::is_key_pressed_once(engine::Key::SPACE)) {
        engine::Entity entity = scene->create_entity("Bob");
        entity.add_component<engine::CameraComponent>();
    }
}

void SandboxLayer::on_render(engine::SystemContext& context, float delta_time) {
    engine::Renderer::clear({0.12f, 0.12f, 0.12f, 1.0f});

    context.renderer.begin_scene(camera_);
    if (auto scene = context.scenes.active_scene()) {
        scene->on_render(context.renderer, camera_, context.assets, delta_time);
    }
    context.renderer.end_scene(delta_time);
    context.renderer.render();
}

void SandboxLayer::on_event(engine::SystemContext& context, engine::Event& event) {
    engine::EventDispatcher dispatcher(event);
    dispatcher.dispatch<engine::WindowResizedEvent>([this](const engine::WindowResizedEvent& e) {
        if (e.height() > 0) {
            camera_.set_aspect_ratio(static_cast<float>(e.width()) / static_cast<float>(e.height()));
        }
        return false;
    });

    dispatcher.dispatch<engine::MouseScrolledEvent>([this](const engine::MouseScrolledEvent& e) {
        engine::CameraController::on_scroll(camera_, e.y_offset());
        return true;
    });
}
