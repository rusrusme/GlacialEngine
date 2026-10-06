//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/debug/ImguiPlatform.hpp"

#include <imgui_impl_glfw.h>

#include "core/WindowAPI.hpp"

#ifdef ENABLE_IMGUI

namespace engine::debug {
    bool init_platform_backend(void* window_handle, GraphicsFramework framework) {
        if (!window_handle) return false;

        switch (framework) {
            case GraphicsFramework::GLFW:
                return ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window_handle), true);
            case GraphicsFramework::SDL:
                // ImGui_ImplSDL2_InitForOpenGL(...);
                return false;
        }
        return false;
    }

    void shutdown_platform_backend(GraphicsFramework framework) {
        switch (framework) {
            case GraphicsFramework::GLFW:
                ImGui_ImplGlfw_Shutdown();
                break;
            case GraphicsFramework::SDL:
                // ImGui_ImplSDL2_Shutdown();
                break;
        }
    }

    void new_frame_platform_backend(GraphicsFramework framework) {
        switch (framework) {
            case GraphicsFramework::GLFW:
                ImGui_ImplGlfw_NewFrame();
                break;
            case GraphicsFramework::SDL:
                // ImGui_ImplSDL2_NewFrame();
                break;
        }
    }
}

#endif
