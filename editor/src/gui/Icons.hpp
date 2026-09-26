//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#ifdef ENABLE_IMGUI
#include <imgui.h>
#endif
#include <algorithm>
#include <string>

namespace editor {
    enum class EntityType { Transform, Mesh2D, Mesh3D, Camera, Light };

    static EntityType get_entity_type_from_name(std::string name) {
        std::ranges::transform(name, name.begin(), [](const unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        if (name.find("camera") != std::string::npos) {
            return EntityType::Camera;
        }
        if (name.find("light") != std::string::npos || name.find("sun") != std::string::npos) {
            return EntityType::Light;
        }
        if (name.find("plane") != std::string::npos || name.find("circle") != std::string::npos) {
            return EntityType::Mesh2D;
        }
        if (name.find("cube") != std::string::npos || name.find("sphere") != std::string::npos ||
            name.find("cylinder") != std::string::npos || name.find("mesh") != std::string::npos) {
            return EntityType::Mesh3D;
        }
        return EntityType::Transform;
    }

    #ifdef ENABLE_IMGUI
    static void draw_entity_icon(ImDrawList* draw_list, const ImVec2 pos, const EntityType type) {
        constexpr float size = 14.0f;
        const auto center = ImVec2(pos.x + size * 0.5f, pos.y + size * 0.5f);

        switch (type) {
            case EntityType::Camera : {
                ImU32 col = IM_COL32(90, 190, 255, 230); // Soft Cyan
                draw_list->AddCircleFilled(ImVec2(pos.x + 3.5f, pos.y + 3.0f), 1.5f, col);
                draw_list->AddCircleFilled(ImVec2(pos.x + 6.5f, pos.y + 3.0f), 1.5f, col);
                draw_list->AddRectFilled(ImVec2(pos.x + 1.0f, pos.y + 4.5f), ImVec2(pos.x + 8.5f, pos.y + 11.5f), col, 1.5f);
                draw_list->AddTriangleFilled(
                    ImVec2(pos.x + 8.5f, pos.y + 6.5f),
                    ImVec2(pos.x + 13.0f, pos.y + 4.0f),
                    ImVec2(pos.x + 13.0f, pos.y + 11.5f),
                    col
                );
                break;
            }
            case EntityType::Light : {
                ImU32 col = IM_COL32(255, 210, 80, 240); // Amber Yellow
                draw_list->AddCircleFilled(ImVec2(center.x, pos.y + 4.5f), 4.0f, col);
                draw_list->AddRectFilled(ImVec2(center.x - 1.2f, pos.y + 11.0f), ImVec2(center.x + 1.2f, pos.y + 12.5f), col, 1.0f);
                break;
            }
            case EntityType::Mesh2D : {
                ImU32 col = IM_COL32(230, 120, 120, 230); // Flat Green Outline Icon
                draw_list->AddRect(ImVec2(pos.x + 2, pos.y + 2), ImVec2(pos.x + 12, pos.y + 12), col, 2.0f, 0, 1.5f);
                draw_list->AddLine(ImVec2(pos.x + 2, pos.y + 2), ImVec2(pos.x + 12, pos.y + 12), col, 1.2f);
                break;
            }
            case EntityType::Mesh3D : {
                ImU32 col = IM_COL32(160, 220, 140, 240); // 3D Isometric Cube
                auto t = ImVec2(center.x, pos.y + 1.5f);
                auto r = ImVec2(pos.x + 12.5f, pos.y + 4.5f);
                auto m = ImVec2(center.x, pos.y + 7.5f);
                auto l = ImVec2(pos.x + 1.5f, pos.y + 4.5f);

                auto b = ImVec2(center.x, pos.y + 13.5f);
                auto br = ImVec2(pos.x + 12.5f, pos.y + 10.5f);
                auto bl = ImVec2(pos.x + 1.5f, pos.y + 10.5f);

                draw_list->AddQuadFilled(t, r, m, l, IM_COL32(160, 220, 140, 70));

                draw_list->AddLine(t, r, col, 1.2f);
                draw_list->AddLine(r, m, col, 1.2f);
                draw_list->AddLine(m, l, col, 1.2f);
                draw_list->AddLine(l, t, col, 1.2f);

                draw_list->AddLine(l, bl, col, 1.2f);
                draw_list->AddLine(m, b, col, 1.2f);
                draw_list->AddLine(r, br, col, 1.2f);

                draw_list->AddLine(bl, b, col, 1.2f);
                draw_list->AddLine(b, br, col, 1.2f);
                break;
            }
            case EntityType::Transform :
            default : {
                ImU32 col = IM_COL32(180, 180, 190, 180); // Neutral Gray
                draw_list->AddLine(ImVec2(center.x - 4, center.y), ImVec2(center.x + 4, center.y), col, 1.5f);
                draw_list->AddLine(ImVec2(center.x, center.y - 4), ImVec2(center.x, center.y + 4), col, 1.5f);
                break;
            }
        }
    }
    #endif

    #ifdef ENABLE_IMGUI
    static void draw_eye_icon(ImDrawList* draw_list, const ImVec2 pos, const bool is_active, const bool is_hovered = false) {
        constexpr float size = 14.0f;
        const auto center = ImVec2(pos.x + size * 0.5f, pos.y + size * 0.5f);

        const ImU32 col = is_active
                              ? (is_hovered ? IM_COL32(240, 240, 245, 255) : IM_COL32(180, 185, 195, 220))
                              : (is_hovered ? IM_COL32(140, 140, 150, 255) : IM_COL32(80, 80, 90, 160));

        // Top Eye Arc
        draw_list->AddBezierCubic(
            ImVec2(pos.x + 1.0f, center.y),
            ImVec2(pos.x + 4.0f, pos.y + 2.0f),
            ImVec2(pos.x + 10.0f, pos.y + 2.0f),
            ImVec2(pos.x + 13.0f, center.y),
            col, 1.2f
        );

        // Bottom Eye Arc
        draw_list->AddBezierCubic(
            ImVec2(pos.x + 1.0f, center.y),
            ImVec2(pos.x + 4.0f, pos.y + 12.0f),
            ImVec2(pos.x + 10.0f, pos.y + 12.0f),
            ImVec2(pos.x + 13.0f, center.y),
            col, 1.2f
        );

        if (is_active) {
            // Pupil
            draw_list->AddCircleFilled(center, 2.2f, col);
        }
        else {
            // Red/Muted Slash across hidden eye
            ImU32 slash_col = is_hovered ? IM_COL32(220, 90, 90, 255) : IM_COL32(160, 70, 70, 200);
            draw_list->AddLine(ImVec2(pos.x + 2.0f, pos.y + 12.0f), ImVec2(pos.x + 12.0f, pos.y + 2.0f), slash_col, 1.5f);
        }
    }
    #endif

    #ifdef ENABLE_IMGUI
    static void draw_sort_icon(ImDrawList* draw_list, const ImVec2 pos, const bool sort_ascending, const ImU32 col = IM_COL32(200, 205, 215, 230)) {
        constexpr float line_right = 7.5f;
        constexpr float arrow_x_offset = 11.5f;

        const float y_top = pos.y + 2.5f;
        const float y_mid = pos.y + 7.0f;
        const float y_bot = pos.y + 11.5f;

        // Line lengths (Right-aligned against arrow spacing)
        const float len_top = sort_ascending ? 6.5f : 2.5f;
        const float len_mid = 4.5f;
        const float len_bot = sort_ascending ? 2.5f : 6.5f;

        constexpr float line_thickness = 1.4f;

        // Draw 3 right-aligned horizontal bars
        draw_list->AddLine(ImVec2(pos.x + line_right - len_top, y_top), ImVec2(pos.x + line_right, y_top), col, line_thickness);
        draw_list->AddLine(ImVec2(pos.x + line_right - len_mid, y_mid), ImVec2(pos.x + line_right, y_mid), col, line_thickness);
        draw_list->AddLine(ImVec2(pos.x + line_right - len_bot, y_bot), ImVec2(pos.x + line_right, y_bot), col, line_thickness);

        const float arrow_x = pos.x + arrow_x_offset;

        // Draw Vertical Arrow shaft & head
        if (sort_ascending) {
            // Arrow UP
            draw_list->AddLine(ImVec2(arrow_x, y_bot), ImVec2(arrow_x, y_top), col, line_thickness);
            draw_list->AddLine(ImVec2(arrow_x - 2.5f, y_top + 2.5f), ImVec2(arrow_x, y_top), col, line_thickness);
            draw_list->AddLine(ImVec2(arrow_x + 2.5f, y_top + 2.5f), ImVec2(arrow_x, y_top), col, line_thickness);
        }
        else {
            // Arrow DOWN
            draw_list->AddLine(ImVec2(arrow_x, y_top), ImVec2(arrow_x, y_bot), col, line_thickness);
            draw_list->AddLine(ImVec2(arrow_x - 2.5f, y_bot - 2.5f), ImVec2(arrow_x, y_bot), col, line_thickness);
            draw_list->AddLine(ImVec2(arrow_x + 2.5f, y_bot - 2.5f), ImVec2(arrow_x, y_bot), col, line_thickness);
        }
    }
    #endif

    #ifdef ENABLE_IMGUI
    static void draw_play_icon(ImDrawList* draw_list, const ImVec2 pos, const ImU32 col = IM_COL32(100, 210, 110, 240)) {
        // Right-pointing filled triangle
        draw_list->AddTriangleFilled(
            ImVec2(pos.x + 3.5f, pos.y + 2.5f),
            ImVec2(pos.x + 3.5f, pos.y + 11.5f),
            ImVec2(pos.x + 11.5f, pos.y + 7.0f),
            col
        );
    }

    static void draw_pause_icon(ImDrawList* draw_list, const ImVec2 pos, const ImU32 col = IM_COL32(255, 190, 60, 240)) {
        // Two vertical parallel bars
        draw_list->AddRectFilled(ImVec2(pos.x + 3.0f, pos.y + 2.5f), ImVec2(pos.x + 5.5f, pos.y + 11.5f), col, 1.0f);
        draw_list->AddRectFilled(ImVec2(pos.x + 8.5f, pos.y + 2.5f), ImVec2(pos.x + 11.0f, pos.y + 11.5f), col, 1.0f);
    }

    static void draw_stop_icon(ImDrawList* draw_list, const ImVec2 pos, const ImU32 col = IM_COL32(230, 80, 80, 240)) {
        // Filled rounded square
        draw_list->AddRectFilled(ImVec2(pos.x + 3.0f, pos.y + 3.0f), ImVec2(pos.x + 11.0f, pos.y + 11.0f), col, 1.5f);
    }

    // Helper to render an ImGui button with a centered vector icon inside it
    static bool draw_icon_button(const char* str_id, void (*draw_icon)(ImDrawList*, ImVec2, ImU32), const ImVec2& size = ImVec2(32, 24)) {
        bool clicked = ImGui::Button(str_id, size);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 btn_min = ImGui::GetItemRectMin();
        ImVec2 btn_size = ImGui::GetItemRectSize();

        constexpr float icon_size = 14.0f;
        ImVec2 icon_pos = ImVec2(
            btn_min.x + (btn_size.x - icon_size) * 0.5f,
            btn_min.y + (btn_size.y - icon_size) * 0.5f
        );

        // Brighten color on hover
        ImU32 col = ImGui::IsItemHovered() ? IM_COL32(255, 255, 255, 255) : IM_COL32(200, 205, 215, 230);
        draw_icon(draw_list, icon_pos, col);

        return clicked;
    }
    #endif
}
