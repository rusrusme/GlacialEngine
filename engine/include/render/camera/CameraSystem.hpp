//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/camera/CameraController.hpp"

namespace engine {
    class Camera;
    class Event;
    class Scene;

    class CameraSystem {
    public:
        void on_update(Scene& active_scene, float delta_time);
        void on_event(Scene& active_scene, Event& event);

    private:
        CameraController controller_;
    };
}
