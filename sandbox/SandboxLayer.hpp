//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "core/Layer.hpp"
#include "render/mesh/Mesh.hpp"
#include "render/pass/MainOpaquePass.hpp"
#include "render/platform/opengl/GLBuffer.hpp"
#include "render/platform/opengl/GLDescriptorSet.hpp"
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
    engine::MainOpaquePass opaque_pass_;
    engine::Unique<engine::GLUniformBuffer> frame_ubo_;
    engine::Unique<engine::GLDescriptorSet> frame_descriptor_set_;

    std::shared_ptr<engine::Mesh> cube_mesh_;
    float camera_angle_ = 0.0f;
};
