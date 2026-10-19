//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"
#include "render/SceneRenderer.hpp"
#include "render/camera/Camera.hpp"
#include "render/camera/CameraController.hpp"

using namespace engine;

class SandboxLayer : public Layer {
public:
    SandboxLayer() : Layer("Sandbox Layer") {}

    void on_attach(SystemContext& context) override;
    void on_detach(SystemContext& context) override;

    void on_update(SystemContext& context, float delta_time) override;
    void on_render(SystemContext& context, float delta_time) override;
    void on_event(SystemContext& context, Event& event) override;

private:
    Camera camera_{45.0f, 16.0f / 9.0f, 0.1f, 100.0f};
    CameraController camera_controller_{};
};
