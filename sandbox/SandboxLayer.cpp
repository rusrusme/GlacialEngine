//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include <memory>

#include "core/Application.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/BufferLayout.hpp"
#include "render/rhi/RenderCommand.hpp"

void SandboxLayer::on_attach(engine::SystemContext& context) {
    // 1. FBO 0 Mock-Framebuffer initialisieren
    engine::FramebufferSpecification fbo_spec{};
    fbo_spec.width = 1280;  // Viewport-Größe für glViewport
    fbo_spec.height = 720;
    fbo_spec.swapchain_target = true; // Setzt intern renderer_id_ = 0

    default_framebuffer_ = std::make_unique<engine::GLFramebuffer>(fbo_spec);

    const std::string shader_src = R"(
    #vert
    #pragma once
    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec4 a_Color;

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

    // 4 Vertices für 2 Dreiecke (Quad) mit Color-Attributen
    float vertices[] = {
        // Position (X, Y, Z)     // Color (R, G, B, A)
        -0.5f, -0.5f, 0.0f, 1.0f, 0.2f, 0.2f, 0.9f, // Unten Links (Rot)
        0.5f, -0.5f, 0.0f, 0.2f, 1.0f, 0.2f, 0.9f,  // Unten Rechts (Grün)
        0.5f, 0.5f, 0.0f, 0.2f, 0.2f, 1.0f, 0.9f,   // Oben Rechts (Blau)
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.2f, 0.9f   // Oben Links (Gelb)
    };

    uint32_t indices[] = {
        0, 1, 2, // Erste Dreieck
        2, 3, 0  // Zweites Dreieck
    };

    vertex_buffer_ = std::make_unique<engine::GLVertexBuffer>(sizeof(vertices), vertices);
    index_buffer_ = std::make_unique<engine::GLIndexBuffer>(sizeof(indices), indices);

    layout_ = {
        {engine::ShaderDataType::Float3, "a_Position"},
        {engine::ShaderDataType::Float4, "a_Color"}
    };

    // Pipeline State Konfiguration testen (CullMode, Blending, Depth, Layout)
    engine::PipelineStateDesc desc{};
    desc.shader = shader_;
    desc.vertex_layout = layout_;
    desc.cull_mode = engine::CullMode::None;
    desc.polygon_mode = engine::PolygonMode::Fill;
    desc.depth_test = false;
    desc.blend_enable = true;
    desc.blend_mode = engine::BlendMode::Alpha;

    pipeline_state_ = std::make_unique<engine::GLPipelineState>(desc);


    frame_ubo_ = std::make_unique<engine::GLUniformBuffer>(sizeof(glm::mat4), 0);

    // Erstelle DescriptorSet und schreibe das UBO an Binding Slot 0
    frame_descriptor_set_ = std::make_unique<engine::GLDescriptorSet>();
    frame_descriptor_set_->write_uniform_buffer(0, *frame_ubo_);
}

void SandboxLayer::on_detach(engine::SystemContext& context) {
}

void SandboxLayer::on_update(engine::SystemContext& context, float delta_time) {
    camera_angle_ += delta_time * 0.5f;
}

void SandboxLayer::on_render(engine::SystemContext& context, float delta_time) {
    constexpr float aspect_ratio = 1280.0f / 720.0f;
    const glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 100.0f);

    // Kamera im Kreis bewegen lassen, schaut auf den Ursprung (0, 0, 0)
    const float cam_x = sin(camera_angle_) * 2.5f;
    const float cam_z = cos(camera_angle_) * 2.5f;
    const glm::mat4 view = glm::lookAt(glm::vec3(cam_x, 1.0f, cam_z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::mat4 view_proj = proj * view;

    // Daten auf die GPU hochladen
    frame_ubo_->set_data(&view_proj, sizeof(glm::mat4));


    engine::RenderCommand::begin_frame();
    auto& cb = engine::RenderCommand::active_command_buffer();

    // TODO Integrate swapchain framebuffer
    constexpr glm::vec4 clear_color = {0.1f, 0.1f, 0.12f, 1.0f};
    cb.begin_render_pass(*default_framebuffer_, clear_color);

    cb.bind_pipeline(*pipeline_state_);

    cb.bind_descriptor_set(*frame_descriptor_set_);

    cb.bind_vertex_buffer(0, *vertex_buffer_, 0, layout_.stride());
    cb.bind_index_buffer(*index_buffer_, engine::IndexType::UInt32);

    cb.draw_indexed(6);

    cb.end_render_pass();
    engine::RenderCommand::end_frame();
}

void SandboxLayer::on_event(engine::SystemContext& context, engine::Event& event) {
    engine::EventDispatcher dispatcher(event);
}
