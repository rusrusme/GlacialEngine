//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/WindowSystem.hpp"

#include <GLFW/glfw3.h>

#include "core/Log.hpp"
#include "event/KeyEvents.hpp"
#include "event/MouseEvents.hpp"
#include "event/WindowEvents.hpp"

////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <GLFW/glfw3native.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "comctl32.lib")
#endif

namespace engine::window {
    #if defined(_WIN32)
    static constexpr UINT_PTR TITLEBAR_SUBCLASS_ID = 1001;

    static LRESULT CALLBACK custom_titlebar_subclass_proc(
        HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
        auto* data = reinterpret_cast<WindowNativeData*>(dwRefData);

        switch (msg) {
            case WM_NCCALCSIZE: {
                if (wparam == TRUE) {
                    auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
                    if (IsZoomed(hwnd)) {
                        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
                        MONITORINFO monitor_info{};
                        monitor_info.cbSize = sizeof(MONITORINFO);
                        if (GetMonitorInfoW(monitor, &monitor_info)) {
                            params->rgrc[0] = monitor_info.rcWork;
                        }
                    }
                    return 0; // Strip standard OS titlebar while preserving Aero Snap
                }
                return 0;
            }

            case WM_NCHITTEST: {
                LRESULT hit = DefSubclassProc(hwnd, msg, wparam, lparam);
                if (hit == HTCLIENT) {
                    POINT pt = {.x = GET_X_LPARAM(lparam), .y = GET_Y_LPARAM(lparam)};
                    ScreenToClient(hwnd, &pt);

                    RECT rc{};
                    GetClientRect(hwnd, &rc);

                    // Native 8-way resize borders when windowed
                    if (!IsZoomed(hwnd)) {
                        const int border = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                        constexpr int top_border = 4;

                        const bool left   = pt.x < border;
                        const bool right  = pt.x >= rc.right - border;
                        const bool top    = pt.y < top_border;
                        const bool bottom = pt.y >= rc.bottom - border;

                        if (top && left)     return HTTOPLEFT;
                        if (top && right)    return HTTOPRIGHT;
                        if (bottom && left)  return HTBOTTOMLEFT;
                        if (bottom && right) return HTBOTTOMRIGHT;
                        if (left)            return HTLEFT;
                        if (right)           return HTRIGHT;
                        if (top)             return HTTOP;
                        if (bottom)          return HTBOTTOM;
                    }

                    // Treat empty ImGui Menubar space as native OS titlebar
                    if (data && data->titlebar_hovered) {
                        return HTCAPTION;
                    }
                }
                return hit;
            }

            case WM_NCMOUSEMOVE: {
                POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                ScreenToClient(hwnd, &pt);
                DefSubclassProc(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
                break;
            }

            case WM_NCDESTROY: {
                RemoveWindowSubclass(hwnd, custom_titlebar_subclass_proc, uIdSubclass);
                break;
            }
            default: break;
        }

        return DefSubclassProc(hwnd, msg, wparam, lparam);
    }

    static void setup_win32_seamless_titlebar(GLFWwindow* window, WindowNativeData* window_data) {
        HWND hwnd = glfwGetWin32Window(window);
        if (!hwnd) return;

        // SetWindowSubclass eliminates wndproc race conditions and property leaks
        SetWindowSubclass(hwnd, custom_titlebar_subclass_proc, TITLEBAR_SUBCLASS_ID,
                          reinterpret_cast<DWORD_PTR>(window_data));

        BOOL dark_mode = TRUE;
        DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark_mode, sizeof(dark_mode));

        constexpr MARGINS margins = {
            .cxLeftWidth    = 1,
            .cxRightWidth   = 1,
            .cyTopHeight    = 1,
            .cyBottomHeight = 1
        };
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }
    #endif
}

////////////////////////////////////////////////////////////////////////

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
        ACTIVE_FRAMEWORK = framework;
        switch (ACTIVE_FRAMEWORK) {
            case GraphicsFramework::GLFW :
                #if defined(_WIN32)
                // Tell Windows DWM not to scale down the window during drag
                SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
                #endif

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
        switch (ACTIVE_FRAMEWORK) {
            case GraphicsFramework::GLFW :
                glfwTerminate();
                break;
            case GraphicsFramework::SDL :
                break;
        }
        initialized = false;
    }

    void poll_events() {
        if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
            glfwPollEvents();
        }
        else {
            CORE_WARN("No valid graphics framework found");
        }
    }

    void swap_buffers(void* window_handle) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwSwapBuffers(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::swap_buffers: {0}", window_handle);
        }
    }

    double time() {
        if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
            return glfwGetTime();
        }
        CORE_WARN("No valid graphics framework found");
        return 0.0;
    }

    void* proc_address(const char* procname) {
        if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
            return reinterpret_cast<void*>(glfwGetProcAddress(procname));
        }
        CORE_WARN("No valid graphics framework found");
        return nullptr;
    }

    void* create(int width, int height, const char* title, WindowNativeData* window_data) {
        if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
            #if defined(DEBUG) || defined(_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            #endif

            GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
            if (window && window_data) {
                setup_callbacks(window, window_data);
                #if defined(_WIN32)
                setup_win32_seamless_titlebar(window, window_data);
                #endif
            }
            return window;
        }
        else {
            CORE_WARN("No valid graphics framework found");
        }
        return nullptr;
    }

    void destroy(void* window_handle) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwDestroyWindow(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::destroy_window: {0}", window_handle);
        }
    }

    void make_context_current(void* window_handle) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwMakeContextCurrent(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::make_context_current: {0}", window_handle);
        }
    }

    void* current_context() {
        if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
            return glfwGetCurrentContext();
        }
        CORE_WARN("No valid graphics framework found");
        return nullptr;
    }

    void setup_callbacks(void* window_handle, WindowNativeData* window_data) {
        if (!window_handle || !window_data || ACTIVE_FRAMEWORK != GraphicsFramework::GLFW) return;

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

    void position(void* window_handle, int x, int y) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwSetWindowPos(static_cast<GLFWwindow*>(window_handle), x, y);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::set_position: {0}", window_handle);
        }
    }

    void position(void* window_handle, int* x, int* y) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwGetWindowPos(static_cast<GLFWwindow*>(window_handle), x, y);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::get_position: {0}", window_handle);
        }
    }

    void size(void* window_handle, int* width, int* height) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwGetWindowSize(static_cast<GLFWwindow*>(window_handle), width, height);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::get_size: {0}", window_handle);
        }
    }

    void framebuffer_size(void* window_handle, int* width, int* height) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwGetFramebufferSize(static_cast<GLFWwindow*>(window_handle), width, height);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::get_framebuffer_size: {0}", window_handle);
        }
    }

    void maximize(void* window_handle) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwMaximizeWindow(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::maximize_window: {0}", window_handle);
        }
    }

    bool maximized(void* window_handle) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                return glfwGetWindowAttrib(static_cast<GLFWwindow*>(window_handle), GLFW_MAXIMIZED) != 0;
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::is_maximized: {0}", window_handle);
        }
        return false;
    }

    void restore(void* window_handle) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwRestoreWindow(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::restore: {0}", window_handle);
        }
    }

    void minimize(void* window_handle) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwIconifyWindow(static_cast<GLFWwindow*>(window_handle));
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::minimize: {0}", window_handle);
        }
    }

    bool minimized(void* window_handle) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
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
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                glfwSetWindowTitle(static_cast<GLFWwindow*>(window_handle), title);
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::set_title: {0}", window_handle);
        }
    }

    void set_vsync(bool enabled) {
        if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
            glfwSwapInterval(enabled ? 1 : 0);
        }
        else {
            CORE_WARN("No valid graphics framework found");
        }
    }

    float monitor_dpi_scale() {
        if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            if (!monitor) return 1.0f;
            float xscale, yscale;
            glfwGetMonitorContentScale(monitor, &xscale, &yscale);
            return xscale;
        }
        CORE_WARN("No valid graphics framework found");
        return 1.0f;
    }

    bool key_pressed(void* window_handle, const int key) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                const int state = glfwGetKey(static_cast<GLFWwindow*>(window_handle), get_glfw_key(key));
                return state == GLFW_PRESS || state == GLFW_REPEAT;
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::is_key_pressed: {0}", window_handle);
        }
        return false;
    }

    bool mouse_button_pressed(void* window_handle, const int button) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
                const int state = glfwGetMouseButton(static_cast<GLFWwindow*>(window_handle), get_glfw_mouse_button(button));
                return state == GLFW_PRESS;
            }
        }
        else {
            CORE_ERROR("Invalid window handle in engine::window::is_mouse_button_pressed: {0}", window_handle);
        }
        return false;
    }

    void mouse_position(void* window_handle, double* xpos, double* ypos) {
        if (window_handle) {
            if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
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
