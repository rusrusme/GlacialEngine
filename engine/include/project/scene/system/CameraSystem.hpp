//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "input/CameraController.hpp"
#include "project/scene/Scene.hpp"

namespace engine {
    class Camera;

    class CameraSystem {
    public:
        void reset_mouse_state();

        void update(const std::shared_ptr<Scene>& scene, float delta_time);

        void on_scroll(const std::shared_ptr<Scene>& scene, double y_offset);

    private:
        CameraController controller_;
    };
}
