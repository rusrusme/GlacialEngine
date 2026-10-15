//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <functional>
#include <string>

#include "event/Event.hpp"

namespace engine {
    enum class GraphicsFramework { GLFW, SDL };

    #if defined(WINDOW_BACKEND_GLFW)
    inline auto ACTIVE_FRAMEWORK = GraphicsFramework::GLFW;
    #elif defined(WINDOW_BACKEND_SDL)
    inline auto ACTIVE_FRAMEWORK = GraphicsFramework::SDL;
    #else
    #error "Unsupported Window Backend"
    #endif

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
        double time();
        void* proc_address(const char* procname);

        void* create(int width, int height, const char* title, WindowNativeData* window_data);
        void destroy(void* window_handle);

        void make_context_current(void* window_handle);
        void* current_context();

        void position(void* window_handle, int x, int y);
        void position(void* window_handle, int* x, int* y);
        void size(void* window_handle, int* width, int* height);
        void framebuffer_size(void* window_handle, int* width, int* height);
        void maximize(void* window_handle);
        bool maximized(void* window_handle);
        bool minimized(void* window_handle);

        void set_title(void* window_handle, const char* title);
        void set_vsync(bool enabled);
        float monitor_dpi_scale();

        bool key_pressed(void* window_handle, int key);
        bool mouse_button_pressed(void* window_handle, int button);
        void mouse_position(void* window_handle, double* xpos, double* ypos);

        void setup_callbacks(void* window_handle, WindowNativeData* window_data);
    }
}
