//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include <memory>

#include "core/Application.hpp"
#include "render/rhi/BufferLayout.hpp"
#include "render/rhi/RenderContext.hpp"

void SandboxLayer::on_attach(engine::SystemContext& context) {
    const std::string shader_src = R"(
    #vert
    #pragma once
    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec3 a_Normal;
    layout(location = 2) in vec2 a_UV;
    layout(location = 3) in vec4 a_Color;

    layout(std140, binding = 0) uniform FrameData {
        mat4 u_ViewProjection;
    };

    layout(location = 0) out vec4 v_Color;

    void main() {
        v_Color = a_Color;
        gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
    }

    #frag
    #pragma once

    layout(location = 0) in vec4 v_Color;
    layout(location = 0) out vec4 FragColor;

    void main() {
        FragColor = v_Color;
    }
    )";

    shader_ = std::make_shared<engine::Shader>("quad_shader", shader_src);

    cube_mesh_ = engine::Mesh::create_cube(context.render_context.device());

    engine::PipelineStateDesc desc{};
    desc.shader = shader_;
    desc.vertex_layout = engine::Vertex::layout();
    desc.cull_mode = engine::CullMode::Back;
    desc.polygon_mode = engine::PolygonMode::Fill;
    desc.depth_test = true;
    desc.blend_enable = false;
    desc.blend_mode = engine::BlendMode::Opaque;

    pipeline_state_ = context.render_context.device().create_pipeline_state(desc);

    frame_ubo_ = context.render_context.device().create_uniform_buffer(sizeof(glm::mat4), 0);
    frame_descriptor_set_ = context.render_context.device().create_descriptor_set();
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

    // Kamera im Kreis bewegen lassen, schaut auf den Ursprung (0, 0, 0)
    const float cam_x = sin(camera_angle_) * 2.5f;
    const float cam_z = cos(camera_angle_) * 2.5f;
    const glm::mat4 view = glm::lookAt(glm::vec3(cam_x, 1.0f, cam_z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::mat4 view_proj = proj * view;

    // Daten auf die GPU hochladen
    frame_ubo_->set_data(&view_proj, sizeof(glm::mat4));

    auto& cb = context.render_context.active_command_buffer();

    constexpr glm::vec4 clear_color = {0.1f, 0.1f, 0.12f, 1.0f};
    cb.begin_render_pass(context.render_context.swapchain().backbuffer(), clear_color);

    cb.bind_pipeline(*pipeline_state_);
    cb.bind_descriptor_set(*frame_descriptor_set_);

    cb.bind_vertex_buffer(0, cube_mesh_->vertex_buffer(), 0, sizeof(engine::Vertex));
    cb.bind_index_buffer(cube_mesh_->index_buffer(), engine::IndexType::UInt32);

    cb.draw_indexed(cube_mesh_->index_count());

    cb.end_render_pass();
}

void SandboxLayer::on_event(engine::SystemContext& context, engine::Event& event) {
    engine::EventDispatcher dispatcher(event);
}
