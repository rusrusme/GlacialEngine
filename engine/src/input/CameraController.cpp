//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "input/CameraController.hpp"
#include "core/Application.hpp"
#include "core/Config.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "input/MouseCodes.hpp"
// #include "render/legacy/Camera.hpp"

namespace engine {
    void CameraController::reset_mouse_state() {
        last_x_ = 0;
        last_y_ = 0;
        is_first_frame_ = true;
    }

    void CameraController::update(Camera& camera, const float delta_time) {
        update_mouse(camera);
        update_keyboard(camera, delta_time);
    }

    void CameraController::update_keyboard(Camera& camera, const float delta_time) {
        if (Input::keyboard_captured()) {
            return;
        }
        if (Input::key_pressed(Key::LEFT_CONTROL) || Input::key_pressed(Key::RIGHT_CONTROL)
            || Input::key_pressed(Key::LEFT_ALT) || Input::key_pressed(Key::RIGHT_ALT)
            || Input::key_pressed(Key::LEFT_SHIFT) || Input::key_pressed(Key::RIGHT_SHIFT)) {
            return;
        }

        float input_x = 0.0f;
        float input_z = 0.0f;

        if (Input::key_pressed(Key::W)) input_z -= 1.0f; // W
        if (Input::key_pressed(Key::A)) input_x += 1.0f; // A
        if (Input::key_pressed(Key::S)) input_z += 1.0f; // S
        if (Input::key_pressed(Key::D)) input_x -= 1.0f; // D

        if (input_x != 0.0f || input_z != 0.0f) {
            // Normalize diagonal movement (so W+A isn't ~41% faster than W alone)
            if (input_x != 0.0f && input_z != 0.0f) {
                input_x *= 0.70710678f; // 1 / sqrt(2)
                input_z *= 0.70710678f;
            }

            // const float effective_distance = std::max(camera.get_distance(), setting::camera_controller::MIN_PAN_DISTANCE);
            // const float dynamic_pan_speed = effective_distance * setting::camera_controller::PAN_SENSITIVITY;
            //
            // const float delta_right = input_x * dynamic_pan_speed * delta_time;
            // const float delta_forward = input_z * dynamic_pan_speed * delta_time;
            // camera.pan(delta_right, delta_forward);
        }
    }

    void CameraController::update_mouse(Camera& camera) {
        // if (Input::is_mouse_captured()) {
        // return;
        // }

        const glm::vec2 mouse_pos = Input::mouse_position();

        if (is_first_frame_) {
            last_x_ = mouse_pos.x;
            last_y_ = mouse_pos.y;
            is_first_frame_ = false;
            return;
        }

        const float delta_x = mouse_pos.x - static_cast<float>(last_x_);
        const float delta_y = mouse_pos.y - static_cast<float>(last_y_);

        last_x_ = mouse_pos.x;
        last_y_ = mouse_pos.y;

        if (Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_MIDDLE)) {
            // camera.orbit(
                // delta_x * setting::camera_controller::ORBIT_SENSITIVITY,
                // delta_y * setting::camera_controller::ORBIT_SENSITIVITY
            // );
        }
    }

    void CameraController::on_scroll(Camera& camera, const double y_offset) {
        // const float dynamic_speed = camera.get_distance() * setting::camera_controller::ZOOM_PERCENTAGE;
        // camera.zoom(static_cast<float>(-y_offset) * dynamic_speed * setting::camera_controller::SCROLL_SPEED);
    }
}
