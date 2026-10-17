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
    const std::string shader_src = R"(
    #vert
    #pragma once

    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec4 a_Color;

    layout(location = 0) out vec4 v_Color;

    void main() {
        v_Color = a_Color;
        gl_Position = vec4(a_Position, 1.0);
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

    float tri_vertices[] = {
        // Position (X, Y, Z)     // Color (R, G, B, A)
        0.2f,  0.2f, 0.0f,       1.0f, 0.0f, 0.0f, 1.0f, // Unten Links (Rot)
        0.8f,  0.2f, 0.0f,       0.0f, 1.0f, 0.0f, 1.0f, // Unten Rechts (Grün)
        0.5f,  0.8f, 0.0f,       0.0f, 0.0f, 1.0f, 1.0f  // Oben Mitte (Blau)
   };

    triangle_vb_ = std::make_unique<engine::GLVertexBuffer>(sizeof(tri_vertices), tri_vertices);

    // GLVertexArray anlegen und VBO direkt binden (ohne PipelineStateDesc)
    triangle_va_ = std::make_unique<engine::GLVertexArray>();
    triangle_va_->add_vertex_buffer(*triangle_vb_, layout_, 0);
}

void SandboxLayer::on_detach(engine::SystemContext& context) {
}

void SandboxLayer::on_update(engine::SystemContext& context, float delta_time) {
}

void SandboxLayer::on_render(engine::SystemContext& context, float delta_time) {
    // engine::RenderCommand::begin_frame();
    engine::RenderCommand::set_clear_color({ 0.1f, 0.1f, 0.12f, 1.0f });
    engine::RenderCommand::clear(true, true);

    auto& cb = engine::RenderCommand::active_command_buffer();

    // -------------------------------------------------------------
    // 1. Quad rendern (über GLPipelineState)
    // -------------------------------------------------------------
    cb.bind_pipeline(*pipeline_state_);
    cb.bind_vertex_buffer(0, *vertex_buffer_, 0, layout_.stride());
    cb.bind_index_buffer(*index_buffer_, engine::IndexType::UInt32);
    cb.draw_indexed(6);

    // -------------------------------------------------------------
    // 2. Separates Dreieck rendern (direkt über GLVertexArray)
    // -------------------------------------------------------------
    shader_->bind();
    triangle_va_->bind(); // VAO hält bereits das VBO-Binding im Speicher
    cb.draw(3);
    // engine::RenderCommand::end_frame();
}

void SandboxLayer::on_event(engine::SystemContext& context, engine::Event& event) {
    engine::EventDispatcher dispatcher(event);
}
