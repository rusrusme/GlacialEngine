//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#ifdef ENABLE_IMGUI
#include <imgui.h>
#endif
#include <core/Application.hpp>

namespace engine {
    class Input {
    public:
        static bool is_mouse_captured() {
            #ifdef ENABLE_IMGUI
            return ImGui::GetIO().WantCaptureMouse;
            #endif
            // Todo: Implement game event handling
            return false;
        }

        static bool is_keyboard_captured() {
            #ifdef ENABLE_IMGUI
            return ImGui::GetIO().WantTextInput;
            #endif
            // Todo: Implement game event handling
            return false;
        }

        static bool is_key_pressed_once(const int key) {
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
    };
}
