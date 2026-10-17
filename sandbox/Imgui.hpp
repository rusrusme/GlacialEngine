//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <imgui.h>

#include "debug/ImguiLayer.hpp"

class Imgui : public engine::ImguiLayer {
public:
    void on_update(engine::SystemContext& context, float delta_time) override {
    }

    void on_imgui_render(engine::SystemContext& context, float delta_time) override {
        static bool show_stats_panel = true;

        if (engine::Input::key_pressed_once(engine::Key::F3)) {
            show_stats_panel = !show_stats_panel;
        }
        if (!show_stats_panel) {
            return;
        }

        ImGui::Begin("Engine Statistics", &show_stats_panel);
        ImGui::Text("Frame Time: %.3f ms", delta_time * 1000.0f);
        ImGui::Text("FPS: %.1f", 1.0f / delta_time);
        ImGui::Separator();

        static float clear_color[3] = {0.1f, 0.1f, 0.1f};
        ImGui::ColorEdit3("Clear Color", clear_color);

        static int click_count = 0;
        if (ImGui::Button("Click Me")) {
            click_count++;
        }
        ImGui::SameLine();
        ImGui::Text("Clicks: %d", click_count);
        ImGui::End();
    }
};
