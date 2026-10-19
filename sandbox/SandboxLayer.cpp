//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include <memory>

#include "core/Application.hpp"
#include "render/rhi/RenderContext.hpp"

void SandboxLayer::on_attach(engine::SystemContext& context) {
    auto& device = context.render_context.device();
    const auto& swapchain = context.render_context.swapchain();

    cube_mesh_ = engine::Mesh::create_cube(device);
    opaque_pass_.init(device);

    test_texture_ = device.create_texture_2d("assets/textures/marble_rock.jpg");

    frame_ubo_ = device.create_uniform_buffer(sizeof(glm::mat4), 0);
    frame_descriptor_set_ = device.create_descriptor_set();
    frame_descriptor_set_->write_uniform_buffer(0, *frame_ubo_);

    camera_.set_viewport_size(swapchain.width(), swapchain.height());
    camera_.set_distance(4.0f);
}

void SandboxLayer::on_detach(engine::SystemContext& context) {
}

void SandboxLayer::on_update(engine::SystemContext& context, float delta_time) {
    camera_controller_.on_update(camera_);
    camera_angle_ += delta_time * 0.5f;
}

void SandboxLayer::on_render(engine::SystemContext& context, float delta_time) {
    const auto& swapchain = context.render_context.swapchain();
    if (swapchain.width() == 0 || swapchain.height() == 0) { return; }

    const glm::mat4& view_proj = camera_.view_projection();
    frame_ubo_->set_data(&view_proj, sizeof(glm::mat4));

    glm::mat4 model_left = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2f, 0.0f, 0.0f));
    glm::mat4 model_right = glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 0.0f, 0.0f));

    // Rotate white cube
    model_right = glm::rotate(model_right, camera_angle_ * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    const engine::RenderObject objects[] = {
        {cube_mesh_.get(), model_left, test_texture_.get()},
        {cube_mesh_.get(), model_right, nullptr}
    };

    opaque_pass_.execute(
        context.render_context.active_command_buffer(),
        context.render_context.swapchain().backbuffer(),
        *frame_descriptor_set_,
        objects
    );
}

void SandboxLayer::on_event(engine::SystemContext& context, engine::Event& event) {
    camera_controller_.on_event(camera_, event);
}
