//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <imgui.h>
#include <glm/vec4.hpp>

namespace editor {
    struct RimeTheme {
        // --- Colors ---
        glm::vec4 canvas_bg = {0.10f, 0.105f, 0.115f, 1.0f}; // #1a1b1e editor & gutter bg
        ImU32 gutter_divider_col = IM_COL32(48, 50, 56, 255);   // Optional vertical gutter border
        ImU32 line_number_col = IM_COL32(100, 105, 115, 255);   // Inactive line numbers
        ImU32 guide_line_col = IM_COL32(75, 78, 86, 255);       // Column hard-wrap guide
        ImU32 caret_col = IM_COL32(230, 235, 245, 255);         // Blinking caret line

        // --- Gutter & Layout Metrics ---
        float gutter_pad_left = 12.0f;
        float gutter_pad_right = 10.0f;
        int gutter_min_digits = 3;

        // --- Interaction & Caret ---
        float scroll_wheel_speed = 4.0f; // Multiplier for mouse-wheel lines scrolled
        float caret_width = 1.5f;
        float caret_blink_rate = 1.0f;   // Period in seconds
        float caret_blink_ratio = 0.55f; // Visible fraction of period
    };
}
