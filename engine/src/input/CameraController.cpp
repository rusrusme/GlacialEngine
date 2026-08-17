//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <core/Config.hpp>
#include <input/CameraController.hpp>
#include <input/Input.hpp>

namespace engine {
    void CameraController::reset_mouse_state() {
        last_x_ = 0;
        last_y_ = 0;
        is_first_frame_ = true;
    }

    void CameraController::update(Camera& camera, const float delta_time) {
        GLFWwindow* window = Application::get().get_window();
        if (!window) return;

        update_mouse(camera);
        update_keyboard(camera, delta_time);
    }

    void CameraController::update_keyboard(Camera& camera, const float delta_time) {
        GLFWwindow* window = Application::get().get_window();
        if (!window) return;

        if (Input::is_keyboard_captured()) {
            return;
        }
        float input_x = 0.0f;
        float input_z = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) input_z -= 1.0f; // W
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) input_x += 1.0f; // A
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) input_z += 1.0f; // S
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) input_x -= 1.0f; // D

        if (input_x != 0.0f || input_z != 0.0f) {
            // Normalize diagonal movement (so W+A isn't ~41% faster than W alone)
            if (input_x != 0.0f && input_z != 0.0f) {
                input_x *= 0.70710678f; // 1 / sqrt(2)
                input_z *= 0.70710678f;
            }

            const float effective_distance = std::max(camera.get_distance(), setting::camera_controller::MIN_PAN_DISTANCE);
            const float dynamic_pan_speed = effective_distance * setting::camera_controller::PAN_SENSITIVITY;

            const float delta_right = input_x * dynamic_pan_speed * delta_time;
            const float delta_forward = input_z * dynamic_pan_speed * delta_time;
            camera.pan(delta_right, delta_forward);
        }
    }

    void CameraController::update_mouse(Camera& camera) {
        GLFWwindow* window = Application::get().get_window();
        if (!window) return;

        double mouse_x = 0.0;
        double mouse_y = 0.0;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        if (is_first_frame_) {
            last_x_ = mouse_x;
            last_y_ = mouse_y;
            is_first_frame_ = false;
            return;
        }

        const double delta_x = mouse_x - last_x_;
        const double delta_y = mouse_y - last_y_;

        last_x_ = mouse_x;
        last_y_ = mouse_y;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
            camera.orbit(
                static_cast<float>(+delta_x) * setting::camera_controller::ORBIT_SENSITIVITY,
                static_cast<float>(+delta_y) * setting::camera_controller::ORBIT_SENSITIVITY
            );
        }
    }

    void CameraController::on_scroll(Camera& camera, const double y_offset) {
        const float dynamic_speed = camera.get_distance() * setting::camera_controller::ZOOM_PERCENTAGE;
        camera.zoom(static_cast<float>(-y_offset) * dynamic_speed * setting::camera_controller::SCROLL_SPEED);
    }
}
