//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "core/Layer.hpp"
#include "render/platform/opengl/GLBuffer.hpp"
#include "render/platform/opengl/GLPipelineState.hpp"
#include "render/rhi/Shader.hpp"

class SandboxLayer : public engine::Layer {
public:
    SandboxLayer() : Layer("Sandbox Layer") {}

    void on_attach(engine::SystemContext& context) override;
    void on_detach(engine::SystemContext& context) override;

    void on_update(engine::SystemContext& context, float delta_time) override;
    void on_render(engine::SystemContext& context, float delta_time) override;
    void on_event(engine::SystemContext& context, engine::Event& event) override;

private:
    std::shared_ptr<engine::Shader> shader_;
    std::unique_ptr<engine::GLVertexBuffer> vertex_buffer_;
    std::unique_ptr<engine::GLIndexBuffer> index_buffer_;
    std::unique_ptr<engine::GLPipelineState> pipeline_state_;
    engine::BufferLayout layout_;

    std::unique_ptr<engine::GLVertexBuffer> triangle_vb_;
};
