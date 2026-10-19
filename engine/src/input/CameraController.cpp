//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "input/CameraController.hpp"

#include "event/Event.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "input/MouseCodes.hpp"
#include "render/StaticCamera.hpp"

namespace engine {
    void CameraController::on_update(StaticCamera& camera, float delta_time) {
        const auto mouse_position = Input::mouse_position();

        if (is_first_frame_) {
            last_mouse_position_ = mouse_position;
            is_first_frame_ = false;
            return;
        }

        const auto delta = mouse_position - last_mouse_position_;
        last_mouse_position_ = mouse_position;

        // Panning with MMB or LSHIFT + RMB
        const bool is_panning = Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_MIDDLE) ||
            (Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_RIGHT) && Input::key_pressed(Key::LEFT_SHIFT));

        if (is_panning) {
            const float dynamic_pan = (pan_speed_ * MAX_PAN_SPEED) * camera.distance();
            camera.pan({-delta.x * dynamic_pan, delta.y * dynamic_pan});
            return;
        }

        // Orbit with RMB
        if (Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_RIGHT)) {
            const float effective_orbit = orbit_speed_ * MAX_ORBIT_SPEED;
            camera.orbit(-delta.x * effective_orbit, -delta.y * effective_orbit);
        }
    }

    void CameraController::on_event(StaticCamera& camera, Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseScrolledEvent>([this, &camera](const MouseScrolledEvent& e) {
            return on_mouse_scrolled(camera, e);
        });
        dispatcher.dispatch<WindowResizedEvent>([this, &camera](const WindowResizedEvent& e) {
            return on_window_resized(camera, e);
        });
    }

    bool CameraController::on_mouse_scrolled(StaticCamera& camera, const MouseScrolledEvent& e) {
        camera.zoom(e.y_offset() * zoom_speed_);
        return false;
    }

    bool CameraController::on_window_resized(StaticCamera& camera, const WindowResizedEvent& e) {
        camera.set_viewport_size(e.width(), e.height());
        return false;
    }
}
