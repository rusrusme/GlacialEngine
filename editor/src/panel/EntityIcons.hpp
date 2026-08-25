//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#ifdef ENABLE_IMGUI
#include <imgui.h>
#endif
#include <algorithm>
#include <cmath>
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
                ImU32 col = IM_COL32(160, 220, 140, 230); // Flat Green Outline Icon
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
}
