//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "project/scene/Scene.hpp"
#include "render/camera/CameraController.hpp"

// TODO Update to new CameraController class
namespace engine {
    class CameraSystem {
    public:
        void on_update(const std::shared_ptr<Scene>& scene, float delta_time);
        void on_event(const std::shared_ptr<Scene>& scene, double y_offset);

    private:
        CameraController controller_;
    };
}
