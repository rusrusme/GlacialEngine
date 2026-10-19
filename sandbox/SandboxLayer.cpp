//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include <memory>

#include "core/Application.hpp"
#include "render/rhi/RenderContext.hpp"

void SandboxLayer::on_attach(SystemContext& context) {
    const auto& device = context.render_context.device();
    const auto& swapchain = context.render_context.swapchain();

    scene_renderer_.init(device);

    const auto cube_handle = context.assets.asset_handle_from_path("primitive://cube");
    cube_mesh_ = context.assets.get<Mesh>(cube_handle);

    const auto texture_handle = context.assets.asset_handle_from_path("assets/textures/marble_rock.jpg");
    test_texture_ = context.assets.get<Texture2D>(texture_handle);

    camera_.set_viewport_size(swapchain.width(), swapchain.height());
    camera_.set_distance(4.0f);
}

void SandboxLayer::on_detach(SystemContext& context) {}

void SandboxLayer::on_update(SystemContext& context, float delta_time) {
    camera_controller_.on_update(camera_);
    rotation_angle_ += delta_time * 0.5f;
}

void SandboxLayer::on_render(SystemContext& context, float delta_time) {
    const auto& swapchain = context.render_context.swapchain();
    if (swapchain.width() == 0 || swapchain.height() == 0) return;

    camera_.set_viewport_size(swapchain.width(), swapchain.height());

    glm::mat4 model_left = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2f, 0.0f, 0.0f));
    glm::mat4 model_right = glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 0.0f, 0.0f));
    model_right = glm::rotate(model_right, rotation_angle_, glm::vec3(0.0f, 1.0f, 0.0f));

    scene_renderer_.begin_scene(camera_);
    scene_renderer_.submit(cube_mesh_.get(), model_left, test_texture_.get());
    scene_renderer_.submit(cube_mesh_.get(), model_right, nullptr);
    scene_renderer_.end_scene(
        context.render_context.active_command_buffer(),
        context.render_context.swapchain().backbuffer()
    );
}

void SandboxLayer::on_event(SystemContext& context, Event& event) {
    camera_controller_.on_event(camera_, event);
}
