//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/WindowAPI.hpp"

#include <GLFW/glfw3.h>

#include "core/Log.hpp"
#include "event/KeyEvents.hpp"
#include "event/MouseEvents.hpp"
#include "event/WindowEvents.hpp"

namespace engine::window {
    static bool initialized = false;

    static int get_glfw_key(const int engine_key) {
        return engine_key;
    }

    static int get_engine_key(const int glfw_key) {
        return glfw_key;
    }

    static int get_glfw_mouse_button(const int engine_button) {
        return engine_button;
    }

    static int get_engine_mouse_button(const int glfw_button) {
        return glfw_button;
    }

    bool init(const GraphicsFramework framework) {
        active_framework = framework;
        switch (active_framework) {
            case GraphicsFramework::GLFW :
                if (!glfwInit()) {
                    CORE_CRITICAL("Failed to initialize GLFW");
                    return false;
                }
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                break;
            case GraphicsFramework::SDL :
                return false;
                break;
        }
        initialized = true;
        return true;
    }

    void shutdown() {
        if (!initialized) {
            CORE_ERROR("Tried to call engine::window::shutdown before initializing");
            return;
        }
        switch (active_framework) {
            case GraphicsFramework::GLFW :
                glfwTerminate();
                break;
            case GraphicsFramework::SDL :
                break;
        }
        initialized = false;
    }

    void poll_events() {
        if (active_framework == GraphicsFramework::GLFW) {
            glfwPollEvents();
        }
        else {
            CORE_WARN("No valid graphics framework found");
        }
    }

    void swap_buffers(void* window_handle) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwSwapBuffers(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::swap_buffers: {0}", window_handle);
        }
    }

    double get_time() {
        if (active_framework == GraphicsFramework::GLFW) {
            return glfwGetTime();
        }
        CORE_WARN("No valid graphics framework found");
        return 0.0;
    }

    void* get_proc_address(const char* procname) {
        if (active_framework == GraphicsFramework::GLFW) {
            return reinterpret_cast<void*>(glfwGetProcAddress(procname));
        }
        CORE_WARN("No valid graphics framework found");
        return nullptr;
    }

    void* create_window(int width, int height, const char* title, WindowNativeData* window_data) {
        if (active_framework == GraphicsFramework::GLFW) {
            GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
            if (window && window_data) {
                setup_callbacks(window, window_data);
            }
            return window;
        }
        else {
            CORE_WARN("No valid graphics framework found");
        }
        return nullptr;
    }

    void destroy_window(void* window_handle) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwDestroyWindow(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::destroy_window: {0}", window_handle);
        }
    }

    void make_context_current(void* window_handle) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwMakeContextCurrent(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::make_context_current: {0}", window_handle);
        }
    }

    void* get_current_context() {
        if (active_framework == GraphicsFramework::GLFW) {
            return glfwGetCurrentContext();
        }
        CORE_WARN("No valid graphics framework found");
        return nullptr;
    }

    void setup_callbacks(void* window_handle, WindowNativeData* window_data) {
        if (!window_handle || !window_data || active_framework != GraphicsFramework::GLFW) return;

        auto* window = static_cast<GLFWwindow*>(window_handle);
        glfwSetWindowUserPointer(window, window_data);

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, const int width, const int height) {
            auto* data = static_cast<WindowNativeData*>(glfwGetWindowUserPointer(win));
            if (data && data->event_callback) {
                data->width = width;
                data->height = height;
                WindowResizedEvent event(width, height);
                data->event_callback(event);
            }
        });

        glfwSetWindowCloseCallback(window, [](GLFWwindow* win) {
            auto* data = static_cast<WindowNativeData*>(glfwGetWindowUserPointer(win));
            if (data && data->event_callback) {
                WindowClosedEvent event;
                data->event_callback(event);
            }
        });

        glfwSetKeyCallback(window, [](GLFWwindow* win, const int key, int scancode, int action, int mods) {
            auto* data = static_cast<WindowNativeData*>(glfwGetWindowUserPointer(win));
            if (!data || !data->event_callback) return;
            const int engine_key = get_engine_key(key);

            switch (action) {
                case GLFW_PRESS : {
                    KeyPressedEvent event(engine_key, false);
                    data->event_callback(event);
                    break;
                }
                case GLFW_RELEASE : {
                    KeyReleasedEvent event(engine_key);
                    data->event_callback(event);
                    break;
                }
                case GLFW_REPEAT : {
                    KeyPressedEvent event(engine_key, true);
                    data->event_callback(event);
                    break;
                }
                default : break;
            }
        });

        glfwSetCharCallback(window, [](GLFWwindow* win, unsigned int keycode) {
            auto* data = static_cast<WindowNativeData*>(glfwGetWindowUserPointer(win));
            if (!data || !data->event_callback) return;
            KeyTypedEvent event(static_cast<int>(keycode));
            data->event_callback(event);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* win, const int button, int action, int mods) {
            auto* data = static_cast<WindowNativeData*>(glfwGetWindowUserPointer(win));
            if (!data || !data->event_callback) return;
            const int engine_button = get_engine_mouse_button(button);

            if (action == GLFW_PRESS) {
                MouseButtonPressedEvent event(engine_button);
                data->event_callback(event);
            }
            else if (action == GLFW_RELEASE) {
                MouseButtonReleasedEvent event(engine_button);
                data->event_callback(event);
            }
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) {
            auto* data = static_cast<WindowNativeData*>(glfwGetWindowUserPointer(win));
            if (!data || !data->event_callback) return;
            MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
            data->event_callback(event);
        });

        glfwSetScrollCallback(window, [](GLFWwindow* win, const double x_offset, const double y_offset) {
            auto* data = static_cast<WindowNativeData*>(glfwGetWindowUserPointer(win));
            if (data && data->event_callback) {
                MouseScrolledEvent event(static_cast<float>(x_offset), static_cast<float>(y_offset));
                data->event_callback(event);
            }
        });
    }

    void set_position(void* window_handle, int x, int y) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwSetWindowPos(static_cast<GLFWwindow*>(window_handle), x, y);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::set_position: {0}", window_handle);
        }
    }

    void get_position(void* window_handle, int* x, int* y) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwGetWindowPos(static_cast<GLFWwindow*>(window_handle), x, y);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::get_position: {0}", window_handle);
        }
    }

    void get_size(void* window_handle, int* width, int* height) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwGetWindowSize(static_cast<GLFWwindow*>(window_handle), width, height);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::get_size: {0}", window_handle);
        }
    }

    void get_framebuffer_size(void* window_handle, int* width, int* height) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwGetFramebufferSize(static_cast<GLFWwindow*>(window_handle), width, height);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::get_framebuffer_size: {0}", window_handle);
        }
    }

    void maximize_window(void* window_handle) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwMaximizeWindow(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::maximize_window: {0}", window_handle);
        }
    }

    bool is_maximized(void* window_handle) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                return glfwGetWindowAttrib(static_cast<GLFWwindow*>(window_handle), GLFW_MAXIMIZED) != 0;
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::is_maximized: {0}", window_handle);
        }
        return false;
    }

    bool is_minimized(void* window_handle) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                return glfwGetWindowAttrib(static_cast<GLFWwindow*>(window_handle), GLFW_ICONIFIED) != 0;
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::is_minimized: {0}", window_handle);
        }
        return false;
    }

    void set_title(void* window_handle, const char* title) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwSetWindowTitle(static_cast<GLFWwindow*>(window_handle), title);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::set_title: {0}", window_handle);
        }
    }

    void set_vsync(bool enabled) {
        if (active_framework == GraphicsFramework::GLFW) {
            glfwSwapInterval(enabled ? 1 : 0);
        }
        else {
            CORE_WARN("No valid graphics framework found");
        }
    }

    float get_monitor_dpi_scale() {
        if (active_framework == GraphicsFramework::GLFW) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            if (!monitor) return 1.0f;
            float xscale, yscale;
            glfwGetMonitorContentScale(monitor, &xscale, &yscale);
            return xscale;
        }
        CORE_WARN("No valid graphics framework found");
        return 1.0f;
    }

    bool is_key_pressed(void* window_handle, const int key) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                const int state = glfwGetKey(static_cast<GLFWwindow*>(window_handle), get_glfw_key(key));
                return state == GLFW_PRESS || state == GLFW_REPEAT;
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::is_key_pressed: {0}", window_handle);
        }
        return false;
    }

    bool is_mouse_button_pressed(void* window_handle, const int button) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                const int state = glfwGetMouseButton(static_cast<GLFWwindow*>(window_handle), get_glfw_mouse_button(button));
                return state == GLFW_PRESS;
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::is_mouse_button_pressed: {0}", window_handle);
        }
        return false;
    }

    void get_mouse_position(void* window_handle, double* xpos, double* ypos) {
        if (window_handle) {
            if (active_framework == GraphicsFramework::GLFW) {
                glfwGetCursorPos(static_cast<GLFWwindow*>(window_handle), xpos, ypos);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::get_mouse_position: {0}", window_handle);
            if (xpos) *xpos = 0.0;
            if (ypos) *ypos = 0.0;
        }
    }
}
