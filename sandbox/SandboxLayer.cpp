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

    const auto cube_mesh_handle = context.assets.asset_handle_from_path("primitive://cube");

    {
        const AssetHandle albedo_handle = context.assets.asset_handle_from_path("assets/textures/brick/rock_wall_13_diff_1k.jpg");
        const AssetHandle normal_handle = context.assets.asset_handle_from_path("assets/textures/brick/rock_wall_13_nor_gl_1k.jpg");
        auto cube = scene->create_entity("Textured Cube");
        auto& transform = cube.get_component<TransformComponent>();
        transform.position = {-1.2f, 0.0f, 0.0f};

        cube.add_component<MeshComponent>(cube_mesh_handle);
        auto& material = cube.add_component<MaterialComponent>();
        material.color = glm::vec4(1.0f);
        material.texture_handle = albedo_handle;
        material.normal_texture_handle = normal_handle;
    }

    {
        auto cube = scene->create_entity("Rotating White Cube");
        auto& transform = cube.get_component<TransformComponent>();
        transform.position = {1.2f, 0.0f, 0.0f};

        cube.add_component<MeshComponent>(cube_mesh_handle);
        cube.add_component<MaterialComponent>(); // texture_handle = 0 -> White
    }

    auto camera = scene->create_entity("Camera");
    auto& cam_comp = camera.add_component<CameraComponent>();
    cam_comp.native.set_far_clip(10);
    cam_comp.native.set_near_clip(0.1);
    cam_comp.native.set_fov(60);
    cam_comp.is_primary = true;

    const auto& swapchain = context.render_context.swapchain();
    // camera_.set_viewport_size(swapchain.width(), swapchain.height());
    // camera_.set_distance(4.0f);
    cam_comp.native.set_viewport_size(swapchain.width(), swapchain.height());
    cam_comp.native.set_distance(4.0f);
}

void SandboxLayer::on_detach(SystemContext& context) {}

void SandboxLayer::on_update(SystemContext& context, float delta_time) {
    // camera_controller_.on_update(camera_);

    auto scene = context.scenes.active_scene();
    if (!scene) return;

    context.scenes.on_update(delta_time);

    if (Input::key_pressed_once(Key::SPACE)) {
        if (context.scenes.scene_state() == SceneState::Edit) {
            context.scenes.on_play();
        }
        else {
            context.scenes.on_stop();
        }
    }

    if (context.scenes.scene_state() == SceneState::Play) {
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

    // camera_.set_viewport_size(swapchain.width(), swapchain.height());
    auto view = active_scene->view<CameraComponent>();
    for (const auto& [entity, cam_comp] : view.each()) {
        if (!cam_comp.is_primary) { continue; }
        cam_comp.native.set_viewport_size(swapchain.width(), swapchain.height());
        break;
    }

    context.renderer.render_scene(active_scene, context, delta_time, nullptr);
}

void SandboxLayer::on_event(SystemContext& context, Event& event) {
    // camera_controller_.on_event(camera_, event);

    if (!event.handled) {
        context.scenes.on_event(event);
    }
}
