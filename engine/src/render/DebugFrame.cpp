//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#ifdef ENABLE_IMGUI
#include "render/DebugFrame.hpp"

namespace engine {
    void DebugFrame::init(GLFWwindow* window, const char* glsl_version) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 2;

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImFont* main_font = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/Inter-VariableFont_opsz,wght.ttf",
            16.0f,
            &config
        );

        // -----------------------------------------
        // Modern Engine Theme
        // -----------------------------------------
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Remove Tab Menu Button
        style.WindowMenuButtonPosition = ImGuiDir_None;

        style.TabBarBorderSize   = 0.0f;
        style.TabBarOverlineSize = 0.0f;
        style.DockingSeparatorSize = 0.0f;

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
        const ImVec4 bg_dark = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
        const ImVec4 bg_mid = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
        const ImVec4 bg_light = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
        const ImVec4 bg_hover = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
        const ImVec4 accent_muted = ImVec4(0.38f, 0.38f, 0.42f, 1.00f); // Muted Steel Accent
        const ImVec4 accent_hover = ImVec4(0.48f, 0.48f, 0.52f, 1.00f);
        const ImVec4 border_color = ImVec4(0.22f, 0.22f, 0.24f, 0.60f);

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
        colors[ImGuiCol_Tab]                = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_TabHovered]         = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
        colors[ImGuiCol_TabActive]          = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_TabUnfocused]       = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
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

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void DebugFrame::shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void DebugFrame::begin_frame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void DebugFrame::end_frame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        const ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(current_context);
        }
    }
}
#endif
