//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#ifdef ENABLE_IMGUI
#include <imgui.h>
#endif

#include <input/Input.hpp>

#include <core/Application.hpp>
#include <GLFW/glfw3.h>

namespace engine {
    bool Input::is_mouse_captured() {
        #ifdef ENABLE_IMGUI
        return ImGui::GetIO().WantCaptureMouse;
        #endif
        return false;
    }

    bool Input::is_keyboard_captured() {
        #ifdef ENABLE_IMGUI
        return ImGui::GetIO().WantTextInput;
        #endif
        return false;
    }

    bool Input::is_mouse_button_pressed(const int key) {
        auto* window = Application::get().window().native();
        const int state = glfwGetMouseButton(window, key);
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::get_mouse_position() {
        auto* window = Application::get().window().native();
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return {static_cast<float>(x), static_cast<float>(y)};
    }

    float Input::get_mouse_x() {
        return get_mouse_position().x;
    }

    float Input::get_mouse_y() {
        return get_mouse_position().y;
    }

    bool Input::is_key_pressed(const int key) {
        auto* window = Application::get().window().native();
        const int state = glfwGetKey(window, key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::is_key_pressed_once(const int key) {
        static std::unordered_map<int, bool> key_states;
        GLFWwindow* window = Application::get().window().native();
        if (!window) return false;

        const bool is_pressed = glfwGetKey(window, key) == GLFW_PRESS;
        bool& was_pressed_last_frame = key_states[key];

        if (is_pressed && !was_pressed_last_frame) {
            was_pressed_last_frame = true;
            return true;
        }

        if (!is_pressed) {
            was_pressed_last_frame = false;
        }

        return false;
    }
}
