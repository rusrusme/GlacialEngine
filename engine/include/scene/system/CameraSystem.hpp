//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <entt/entity/registry.hpp>

#include "input/CameraController.hpp"

namespace engine {
    class Camera;


    class CameraSystem {
    public:
        void reset_mouse_state();

        void update(entt::registry& registry, float delta_time);

        static void on_scroll(entt::registry& registry, double y_offset);

    private:
        CameraController controller_;
    };
}
