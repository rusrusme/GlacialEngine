//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"
#include "scene/system/CameraSystem.hpp"

class GameLayer : public engine::Layer {
public:
    void on_attach(engine::SystemContext& context) override;
    void on_detach(engine::SystemContext& context) override;
    void on_update(engine::SystemContext& context, float delta_time) override;
    void on_render(engine::SystemContext& context, float delta_time) override;
    void on_event(engine::SystemContext& context, engine::Event& event) override;

private:
    engine::CameraSystem camera_system_;
};
