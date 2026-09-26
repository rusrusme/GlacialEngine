//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "input/Input.hpp"
#include "core/Application.hpp"

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
        return window::is_mouse_button_pressed(window, key);
    }

    glm::vec2 Input::get_mouse_position() {
        auto* window = Application::get().window().native();
        double x, y;
        window::get_mouse_position(window, &x, &y);
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
        return window::is_key_pressed(window, key);
    }

    bool Input::is_key_pressed_once(const int key) {
        static std::unordered_map<int, bool> key_states;
        void* window = Application::get().window().native();
        if (!window) return false;

        const bool is_pressed = window::is_key_pressed(window, key);
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
