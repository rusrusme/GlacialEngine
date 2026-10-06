//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"
#include "input/CameraController.hpp"
#include "render/legacy/Camera.hpp"

class SandboxLayer : public engine::Layer {
public:
    SandboxLayer() : Layer("Sandbox Layer") {}

    void on_attach(engine::SystemContext& context) override;
    void on_detach(engine::SystemContext& context) override;

    void on_update(engine::SystemContext& context, float delta_time) override;
    void on_render(engine::SystemContext& context, float delta_time) override;
    void on_event(engine::SystemContext& context, engine::Event& event) override;

private:
    engine::Camera camera_;
    engine::CameraController camera_controller_;
};
