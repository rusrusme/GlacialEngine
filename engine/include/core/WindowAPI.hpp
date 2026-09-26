//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "event/Event.hpp"

namespace engine {
    enum class GraphicsFramework {
        GLFW,
        SDL
    };

    inline auto active_framework = GraphicsFramework::GLFW;

    using EventCallbackFunc = std::function<void(Event&)>;

    struct WindowNativeData {
        std::string title;
        int width = 0;
        int height = 0;
        EventCallbackFunc event_callback = nullptr;
    };

    namespace window {
        bool init(GraphicsFramework framework = GraphicsFramework::GLFW);
        void shutdown();
        void poll_events();
        void swap_buffers(void* window_handle);
        double get_time();

        void* create_window(int width, int height, const char* title, WindowNativeData* window_data);
        void destroy_window(void* window_handle);

        void make_context_current(void* window_handle);
        void* get_current_context();

        void set_position(void* window_handle, int x, int y);
        void get_position(void* window_handle, int* x, int* y);
        void get_size(void* window_handle, int* width, int* height);
        void get_framebuffer_size(void* window_handle, int* width, int* height);
        void maximize_window(void* window_handle);
        bool is_maximized(void* window_handle);
        bool is_minimized(void* window_handle);

        void set_title(void* window_handle, const char* title);
        void set_vsync(bool enabled);
        float get_monitor_dpi_scale();

        bool is_key_pressed(void* window_handle, int key);
        bool is_mouse_button_pressed(void* window_handle, int button);
        void get_mouse_position(void* window_handle, double* xpos, double* ypos);

        void setup_callbacks(void* window_handle, WindowNativeData* window_data);
    }
}
