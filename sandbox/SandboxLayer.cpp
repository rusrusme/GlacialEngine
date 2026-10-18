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

    cube_mesh_ = engine::Mesh::create_cube(device);
    opaque_pass_.init(device);

    frame_ubo_ = device.create_uniform_buffer(sizeof(glm::mat4), 0);
    frame_descriptor_set_ = device.create_descriptor_set();
    frame_descriptor_set_->write_uniform_buffer(0, *frame_ubo_);
}

void SandboxLayer::on_detach(engine::SystemContext& context) {
}

void SandboxLayer::on_update(engine::SystemContext& context, float delta_time) {
    camera_angle_ += delta_time * 0.5f;
}

void SandboxLayer::on_render(engine::SystemContext& context, float delta_time) {
    const auto& swapchain = context.render_context.swapchain();
    if (swapchain.width() == 0 || swapchain.height() == 0) { return; }

    const float aspect_ratio = static_cast<float>(swapchain.width()) / static_cast<float>(swapchain.height());
    const glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 100.0f);

    const float cam_x = sin(camera_angle_) * 2.5f;
    const float cam_z = cos(camera_angle_) * 2.5f;
    const glm::mat4 view = glm::lookAt(glm::vec3(cam_x, 1.0f, cam_z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::mat4 view_proj = proj * view;
    frame_ubo_->set_data(&view_proj, sizeof(glm::mat4));

    glm::mat4 model_left = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2f, 0.0f, 0.0f));
    glm::mat4 model_right = glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 0.0f, 0.0f));
    model_right = glm::rotate(model_right, camera_angle_ * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    const engine::RenderObject objects[] = {
        {cube_mesh_.get(), model_left},
        {cube_mesh_.get(), model_right}
    };

    opaque_pass_.execute(
        context.render_context.active_command_buffer(),
        context.render_context.swapchain().backbuffer(),
        *frame_descriptor_set_,
        objects
    );
}

void SandboxLayer::on_event(engine::SystemContext& context, engine::Event& event) {
    engine::EventDispatcher dispatcher(event);
}
