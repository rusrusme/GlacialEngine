//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    class Event;
    class Scene;

    class CameraControllerSystem {
    public:
        void on_update(Scene& active_scene, float delta_time);
        void on_event(Scene& active_scene, Event& event);
    };
}
