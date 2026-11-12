//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <algorithm>
#include <glm/glm.hpp>

#include "event/MouseEvents.hpp"
#include "event/WindowEvents.hpp"

namespace engine {
    class Camera;
    class CameraSystem;
    class Event;

    class CameraController {
    public:
        void on_update(Camera& camera);
        void on_event(Camera& camera, Event& event);

        void reset() { is_first_frame_ = true; }

        void set_orbit_speed(float speed) { orbit_speed_ = std::clamp(speed, 0.1f, 1.0f); }
        void set_pan_speed(float speed) { pan_speed_ = std::clamp(speed, 0.1f, 1.0f); }
        void set_zoom_speed(float speed) { zoom_speed_ = std::clamp(speed, 0.1f, 1.0f); }

    private:
        friend CameraSystem;

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
        static constexpr float MAX_ZOOM_SPEED = 2.0f;  // 0.5f * 2.0f = 1.0 Units per Scroll
    };
}
