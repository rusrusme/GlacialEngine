//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

#include "event/MouseEvents.hpp"
#include "event/WindowEvents.hpp"

namespace engine {
    class Event;
    class Camera;

    class CameraController {
    public:
        void on_update(Camera& camera);
        void on_event(Camera& camera, Event& event);

    private:
        bool on_mouse_scrolled(Camera& camera, const MouseScrolledEvent& e);
        bool on_window_resized(Camera& camera, const WindowResizedEvent& e);

        glm::vec2 last_mouse_position_{0.0f};
        bool is_first_frame_{true};

        // Normalized (Default: 0.5f)
        float orbit_speed_{0.5f};
        float pan_speed_{0.5f};
        float zoom_speed_{0.5f};

        static constexpr float MAX_ORBIT_SPEED = 0.5f; // 0.5f * 0.5f = 0.25 Degrees per Pixel
        static constexpr float MAX_PAN_SPEED = 0.002f; // 0.5f * 0.002f = 0.001
        static constexpr float MAX_ZOOM_SPEED = 1.0f;  // 0.5f * 1.0f = 0.5 Units per Scroll
    };
}
