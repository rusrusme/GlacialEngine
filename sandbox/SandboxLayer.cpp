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

void SandboxLayer::on_attach(SystemContext& context) {
    std::shared_ptr<Scene> scene = context.scenes.create("Sandbox Scene");

    const auto tree_handle = context.assets.asset_handle_from_path("assets/models/Tree.glb");

    auto tree = scene->create_entity("Tree");
    tree.add_component<MeshComponent>(tree_handle);
    tree.add_component<MaterialComponent>();

    const auto& swapchain = context.render_context.swapchain();
    camera_.set_viewport_size(swapchain.width(), swapchain.height());
    camera_.set_distance(4.0f);
}

void SandboxLayer::on_detach(SystemContext& context) {}

void SandboxLayer::on_update(SystemContext& context, float delta_time) {
    camera_controller_.on_update(camera_);

    context.scenes.on_update(delta_time);

    if (Input::key_pressed_once(Key::SPACE)) {
        if (context.scenes.scene_state() == SceneState::Edit) {
            context.scenes.on_play();
        }
        else {
            context.scenes.on_stop();
        }
    }

    const auto scene = context.scenes.active_scene();
    if (scene && context.scenes.scene_state() == SceneState::Play) {
        const auto view = scene->view<LabelComponent, TransformComponent>(exclude<DisabledComponent>);
        for (const auto& [entity, label, transform] : view.each()) {
            if (label.tag == "Rotating White Cube") {
                transform.rotation.y += delta_time * 45.0f; // 45 Grad pro Sekunde
            }
        }
    }
}

void SandboxLayer::on_render(SystemContext& context, float delta_time) {
    const auto& swapchain = context.render_context.swapchain();
    if (swapchain.width() == 0 || swapchain.height() == 0) return;

    auto active_scene = context.scenes.active_scene();
    if (!active_scene) return;

    camera_.set_viewport_size(swapchain.width(), swapchain.height());

    context.renderer.render_scene(*active_scene, context, &camera_);
}

void SandboxLayer::on_event(SystemContext& context, Event& event) {
    camera_controller_.on_event(camera_, event);
}
