//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "debug/ImguiPlatform.hpp"

#ifdef ENABLE_IMGUI

#if defined(WINDOW_BACKEND_GLFW)
#include <imgui_impl_glfw.h>
#elif defined(WINDOW_BACKEND_SDL)
#include <imgui_impl_sdl3.h>
#endif

#endif

#include "core/WindowSystem.hpp"

namespace engine::debug {
    bool init_platform_backend(void* window_handle, GraphicsFramework framework) {
        #ifdef ENABLE_IMGUI
        if (!window_handle) return false;

        switch (framework) {
            case GraphicsFramework::GLFW :
                return ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window_handle), true);
            case GraphicsFramework::SDL :
                return false;
        }
        #endif
        return false;
    }

    void shutdown_platform_backend(GraphicsFramework framework) {
        #ifdef ENABLE_IMGUI
        switch (framework) {
            case GraphicsFramework::GLFW :
                ImGui_ImplGlfw_Shutdown();
                break;
            case GraphicsFramework::SDL :
                // ImGui_ImplSDL2_Shutdown();
                break;
        }
        #endif
    }

    void new_frame_platform_backend(GraphicsFramework framework) {
        #ifdef ENABLE_IMGUI
        switch (framework) {
            case GraphicsFramework::GLFW :
                ImGui_ImplGlfw_NewFrame();
                break;
            case GraphicsFramework::SDL :
                // ImGui_ImplSDL2_NewFrame();
                break;
        }
        #endif
    }
}
