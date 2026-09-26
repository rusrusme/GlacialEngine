//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Toolbar.hpp"

#ifdef ENABLE_IMGUI
#include "imgui_internal.h"
#endif

#include "EditorLayer.hpp"
#include "gui/Icons.hpp"
#include "scene/SceneState.hpp"

namespace editor {
    Toolbar::Toolbar() = default;
    Toolbar::~Toolbar() = default;

    #ifdef ENABLE_IMGUI
    void Toolbar::render_imgui() {
        constexpr float slot_width = 36.0f;
        constexpr float slot_height = 24.0f;
        const float spacing_x = ImGui::GetStyle().ItemSpacing.x;

        // Determine minimum required slots
        size_t min_required_slots = 0;
        for (size_t i = 0; i < toolbar_slots_.size(); ++i) {
            if (toolbar_slots_[i].item.has_value()) {
                min_required_slots = i + 1;
            }
        }
        min_required_slots = std::max<size_t>(1, min_required_slots);

        // Window width constraint based on min slots
        constexpr float window_padding_x = 16.0f;
        const float min_window_width
            = static_cast<float>(min_required_slots) * slot_width
            + static_cast<float>(min_required_slots - 1) * spacing_x
            + window_padding_x + 16.0f;

        // Lock window max height to strictly 1 row height without titlebar overhead
        constexpr float window_padding_y = 8.0f;
        const float fixed_window_height = slot_height + window_padding_y;

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(min_window_width, fixed_window_height),
            ImVec2(FLT_MAX, fixed_window_height)
        );

        // Hide tab bar when docked, and lock node flags
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoResize
            | ImGuiDockNodeFlags_NoSplit
            | ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse
            | ImGuiWindowFlags_NoTitleBar;

        if (ImGui::Begin("Toolbar", nullptr, flags)) {
            const ImVec2 avail = ImGui::GetContentRegionAvail();

            auto target_slot_count = static_cast<size_t>(
                std::floor((avail.x + spacing_x) / (slot_width + spacing_x))
            );
            target_slot_count = std::max(target_slot_count, min_required_slots);

            if (target_slot_count != toolbar_slots_.size()) {
                toolbar_slots_.resize(target_slot_count);
                for (size_t i = 0; i < toolbar_slots_.size(); ++i) {
                    if (toolbar_slots_[i].id.empty()) {
                        toolbar_slots_[i].id = "slot_" + std::to_string(i);
                    }
                }
            }

            // Center vertically
            const float center_y = ImGui::GetCursorPosY() + std::max(0.0f, (avail.y - slot_height) * 0.5f);
            ImGui::SetCursorPosY(center_y);

            // Center horizontally so start margin equals end margin
            const float total_slots_width
                = slot_width * static_cast<float>(toolbar_slots_.size())
                + spacing_x * static_cast<float>(toolbar_slots_.size() - 1);

            if (avail.x > total_slots_width) {
                const float symmetric_offset = (avail.x - total_slots_width) * 0.5f;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + symmetric_offset);
            }

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            for (size_t i = 0; i < toolbar_slots_.size(); ++i) {
                if (i > 0) ImGui::SameLine();

                auto& [id, item] = toolbar_slots_[i];
                ImGui::PushID(id.c_str());

                ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                ImVec2 slot_max = {cursor_pos.x + slot_width, cursor_pos.y + slot_height};

                // Render widget if occupied, otherwise invisible dummy space
                if (item.has_value()) {
                    item->render_func(ImVec2(slot_width, slot_height));

                    // Drag source
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("TOOLBAR_SLOT_SWAP", &i, sizeof(size_t));
                        ImGui::Text("Move: %s", item->label.c_str());
                        ImGui::EndDragDropSource();
                    }
                }
                else {
                    ImGui::Dummy(ImVec2(slot_width, slot_height));
                }

                // Drag target
                if (ImGui::BeginDragDropTarget()) {
                    if (!item.has_value()) {
                        draw_list->AddRect(cursor_pos, slot_max, IM_COL32(255, 200, 50, 230), 3.0f, 0, 1.5f);
                    }

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TOOLBAR_SLOT_SWAP", ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
                        size_t source_idx = *static_cast<const size_t*>(payload->Data);
                        if (source_idx != i) {
                            std::swap(toolbar_slots_[source_idx].item, toolbar_slots_[i].item);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
            }

            // Enable dragging/undocking when clicking empty space or empty slots
            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                if (!ImGui::IsAnyItemActive()) {
                    ImGuiWindow* window = ImGui::GetCurrentWindow();
                    if (ImGui::IsWindowDocked()) {
                        ImGuiContext* g = ImGui::GetCurrentContext();
                        ImGui::DockContextProcessUndockWindow(g, window);
                    }
                    ImGui::StartMouseMovingWindow(window);
                }
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
    #endif

    void Toolbar::register_default_items(EditorLayer& editor_layer) {
        toolbar_slots_.clear();
        toolbar_slots_.resize(4);

        for (size_t i = 0; i < toolbar_slots_.size(); ++i) {
            toolbar_slots_[i].id = "slot_" + std::to_string(i);
        }

        toolbar_slots_[0].item = ToolbarItem{
            "play_stop", "Play / Stop", 36.0f,
            [&editor_layer](ImVec2 size) {
                const bool is_play = (editor_layer.get_scene_state() == engine::SceneState::Play);
                const bool is_pause = (editor_layer.get_scene_state() == engine::SceneState::Pause);

                if (is_play || is_pause) {
                    if (draw_icon_button("##StopBtn", draw_stop_icon, size)) editor_layer.on_scene_stop();
                }
                else {
                    if (draw_icon_button("##PlayBtn", draw_play_icon, size)) editor_layer.on_scene_play();
                }
            }
        };

        toolbar_slots_[1].item = ToolbarItem{
            "pause_resume", "Pause / Resume", 36.0f,
            [&editor_layer](ImVec2 size) {
                const bool is_play = (editor_layer.get_scene_state() == engine::SceneState::Play);
                const bool is_pause = (editor_layer.get_scene_state() == engine::SceneState::Pause);

                if (draw_icon_button("##PauseBtn", is_pause ? draw_play_icon : draw_pause_icon, size)) {
                    if (is_play) editor_layer.on_scene_pause();
                    else if (is_pause) editor_layer.on_scene_play();
                }
            }
        };
    }
}
