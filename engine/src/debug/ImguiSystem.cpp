//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#ifdef ENABLE_IMGUI
#include "debug/ImguiSystem.hpp"

#if defined(WINDOW_BACKEND_GLFW)
#include <imgui_impl_glfw.h>
#elif defined(WINDOW_BACKEND_SDL)
#include <imgui_impl_sdl3.h>
#endif

#if defined(RENDER_API_OPENGL)
#include <imgui_impl_opengl3.h>
#elif defined(RENDER_API_VULKAN)
#include <imgui_impl_vulkan.h>
#endif

#include "core/Application.hpp"
#include "debug/ImguiPlatform.hpp"

namespace engine {
    void ImguiSystem::on_attach(void* window, const char* glsl_version) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 2;

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        const float dpiScale = Application::get().window().monitor_dpi_scale();

        io.Fonts->AddFontFromFileTTF(
            "assets/fonts/Inter-VariableFont.ttf",
            16.0f * dpiScale,
            &config
        );

        ImGui::GetStyle().ScaleAllSizes(dpiScale);

        // -----------------------------------------
        // Modern Engine Theme
        // -----------------------------------------
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Remove Tab Menu Button
        style.WindowMenuButtonPosition = ImGuiDir_None;

        style.TabBarBorderSize = 0.0f;
        style.TabBarOverlineSize = 0.0f;
        style.DockingSeparatorSize = 1.0f;

        // Geometry & Spacing
        style.WindowRounding = 6.0f;
        style.ChildRounding = 5.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 3.0f;
        style.TabRounding = 4.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;

        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(8.0f, 5.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
        style.IndentSpacing = 18.0f;
        style.ScrollbarSize = 12.0f;

        // Color Palette (Neutral Charcoal & Warm Gray)
        constexpr auto bg_dark = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
        constexpr auto bg_mid = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
        constexpr auto bg_light = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
        constexpr auto bg_hover = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
        constexpr auto accent_muted = ImVec4(0.38f, 0.38f, 0.42f, 1.00f); // Muted Steel Accent
        constexpr auto accent_hover = ImVec4(0.48f, 0.48f, 0.52f, 1.00f);
        constexpr auto border_color = ImVec4(0.22f, 0.22f, 0.24f, 0.60f);

        // Text & Base
        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.45f, 0.48f, 1.00f);
        colors[ImGuiCol_WindowBg] = bg_dark;
        colors[ImGuiCol_ChildBg] = bg_mid;
        colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.13f, 0.96f);
        colors[ImGuiCol_Border] = border_color;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        // Inputs & Frames
        colors[ImGuiCol_FrameBg] = bg_light;
        colors[ImGuiCol_FrameBgHovered] = bg_hover;
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);

        // Title Bars
        colors[ImGuiCol_TitleBg] = bg_dark;
        colors[ImGuiCol_TitleBgActive] = bg_dark;
        colors[ImGuiCol_TitleBgCollapsed] = bg_dark;
        colors[ImGuiCol_MenuBarBg] = bg_dark;

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);

        // Controls & Buttons
        colors[ImGuiCol_Button] = bg_light;
        colors[ImGuiCol_ButtonHovered] = bg_hover;
        colors[ImGuiCol_ButtonActive] = accent_muted;
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 0.80f);
        colors[ImGuiCol_HeaderHovered] = bg_hover;
        colors[ImGuiCol_HeaderActive] = accent_muted;

        // Sliders & Checkmarks
        colors[ImGuiCol_CheckMark] = ImVec4(0.75f, 0.75f, 0.78f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.32f, 0.32f, 0.35f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = accent_muted;

        // Separators & Grips
        colors[ImGuiCol_Separator] = border_color;
        colors[ImGuiCol_SeparatorHovered] = accent_muted;
        colors[ImGuiCol_SeparatorActive] = accent_hover;
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.22f, 0.22f, 0.24f, 0.30f);
        colors[ImGuiCol_ResizeGripHovered] = accent_muted;
        colors[ImGuiCol_ResizeGripActive] = accent_hover;

        // Docking
        colors[ImGuiCol_DockingPreview] = ImVec4(0.38f, 0.38f, 0.42f, 0.35f);
        colors[ImGuiCol_DockingEmptyBg] = bg_dark;
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.38f, 0.38f, 0.42f, 0.35f);
        colors[ImGuiCol_NavHighlight] = accent_muted;
        // -----------------------------------------

        #if defined(RENDER_API_OPENGL)
        debug::init_platform_backend(window, GraphicsFramework::GLFW);
        ImGui_ImplOpenGL3_Init(glsl_version);
        #elif defined(RENDER_API_VULKAN)
        // ImGui_ImplVulkan_Init(...);
        #endif
    }

    void ImguiSystem::on_detach() {
        #if defined(RENDER_API_OPENGL)
        ImGui_ImplOpenGL3_Shutdown();
        #elif defined(RENDER_API_VULKAN)
        // ImGui_ImplVulkan_Shutdown();
        #endif

        #if defined(WINDOW_BACKEND_GLFW)
        debug::shutdown_platform_backend(GraphicsFramework::GLFW);
        #elif defined(WINDOW_BACKEND_SDL)
        debug::shutdown_platform_backend(GraphicsFramework::SDL);
        #endif

        ImGui::DestroyContext();
    }

    void ImguiSystem::begin_frame() {
        #if defined(RENDER_API_OPENGL)
        ImGui_ImplOpenGL3_NewFrame();
        #elif defined(RENDER_API_VULKAN)
        // Imgui_ImplVulkan_NewFrame()...
        #endif

        #if defined(WINDOW_BACKEND_GLFW)
        debug::new_frame_platform_backend(GraphicsFramework::GLFW);
        #elif defined(WINDOW_BACKEND_SDL)
        debug::new_frame_platform_backend(GraphicsFramework::SDL);
        #endif

        ImGui::NewFrame();
    }

    void ImguiSystem::end_frame() {
        ImGui::Render();

        #if defined(RENDER_API_OPENGL)
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #elif defined(RENDER_API_VULKAN)
        // ...
        #endif

        const ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            void* current_context = window::current_context();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            window::make_context_current(current_context);
        }
    }
}

#endif
