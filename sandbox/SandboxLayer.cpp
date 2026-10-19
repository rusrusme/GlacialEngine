//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include "core/Application.hpp"
#include "render/rhi/RenderContext.hpp"

void SandboxLayer::on_attach(SystemContext& context) {
    const auto& device = context.render_context.device();
    const auto& swapchain = context.render_context.swapchain();

    scene_renderer_.init(device);

    const auto cube_mesh_handle = context.assets.asset_handle_from_path("primitive://cube");
    const auto texture_handle   = context.assets.asset_handle_from_path("assets/textures/marble_rock.jpg");

    {
        auto left_cube = scene_.create_entity("TexturedCube");
        auto& transform = left_cube.get_component<TransformComponent>();
        transform.position = {-1.2f, 0.0f, 0.0f};

        left_cube.add_component<MeshComponent>(cube_mesh_handle);

        auto& material = left_cube.add_component<MaterialComponent>();
        material.texture_handle = texture_handle;
    }

    {
        rotating_cube_ = scene_.create_entity("RotatingWhiteCube");
        auto& transform = rotating_cube_.get_component<TransformComponent>();
        transform.position = {1.2f, 0.0f, 0.0f};

        rotating_cube_.add_component<MeshComponent>(cube_mesh_handle);
        rotating_cube_.add_component<MaterialComponent>(); // texture_handle = 0 -> White
    }

    camera_.set_viewport_size(swapchain.width(), swapchain.height());
    camera_.set_distance(4.0f);
}

void SandboxLayer::on_detach(SystemContext& context) {
    scene_.clear();
}

void SandboxLayer::on_update(SystemContext& context, float delta_time) {
    camera_controller_.on_update(camera_);
    if (rotating_cube_) {
        auto& transform = rotating_cube_.get_component<engine::TransformComponent>();
        transform.rotation.y += delta_time * 45.0f; // 45 Grad pro Sekunde
    }
}

void SandboxLayer::on_render(SystemContext& context, float delta_time) {
    const auto& swapchain = context.render_context.swapchain();
    if (swapchain.width() == 0 || swapchain.height() == 0) return;

    camera_.set_viewport_size(swapchain.width(), swapchain.height());

    scene_renderer_.render_scene(scene_, context, &camera_);
}

void SandboxLayer::on_event(SystemContext& context, Event& event) {
    camera_controller_.on_event(camera_, event);
}
