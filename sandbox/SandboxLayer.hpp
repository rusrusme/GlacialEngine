//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "core/Layer.hpp"
#include "input/CameraController.hpp"
#include "render/StaticCamera.hpp"
#include "render/mesh/Mesh.hpp"
#include "render/pass/MainOpaquePass.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/Texture.hpp"

class SandboxLayer : public engine::Layer {
public:
    SandboxLayer() : Layer("Sandbox Layer") {}

    void on_attach(engine::SystemContext& context) override;
    void on_detach(engine::SystemContext& context) override;

    void on_update(engine::SystemContext& context, float delta_time) override;
    void on_render(engine::SystemContext& context, float delta_time) override;
    void on_event(engine::SystemContext& context, engine::Event& event) override;

private:
    engine::StaticCamera camera_;
    engine::CameraController camera_controller_;

    engine::MainOpaquePass opaque_pass_;
    engine::Unique<engine::GLUniformBuffer> frame_ubo_;
    engine::Unique<engine::GLDescriptorSet> frame_descriptor_set_;

    std::shared_ptr<engine::Mesh> cube_mesh_;
    float camera_angle_ = 0.0f;
    engine::Unique<engine::Texture2D> test_texture_;
};
