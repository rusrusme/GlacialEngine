//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "panel/AssetBrowser.hpp"

namespace editor {
    void AssetBrowser::init(std::shared_ptr<engine::AssetManager> asset_manager) {
        asset_manager_ = std::move(asset_manager);
    }

    #ifdef ENABLE_IMGUI
    void AssetBrowser::render_imgui() {
        ImGui::Begin("Asset Browser");
        render_header();
        render_grid();
        ImGui::End();
    }

    static void draw_item_icon(ImDrawList* draw_list, const ImVec2 p0, const ImVec2 p1, const bool is_directory) {
        const auto center = ImVec2((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);

        if (is_directory) {
            // Folder Tab + Body (Muted Amber)
            ImU32 folder_col = IM_COL32(210, 170, 90, 220);
            draw_list->AddRectFilled(ImVec2(center.x - 18, center.y - 14), ImVec2(center.x - 4, center.y - 10), folder_col, 2.0f);
            draw_list->AddRectFilled(ImVec2(center.x - 18, center.y - 10), ImVec2(center.x + 18, center.y + 12), folder_col, 3.0f);
        }
        else {
            // Document Paper (Slate White)
            ImU32 file_col = IM_COL32(180, 185, 195, 200);
            draw_list->AddRectFilled(ImVec2(center.x - 12, center.y - 15), ImVec2(center.x + 12, center.y + 15), file_col, 2.0f);
            draw_list->AddLine(ImVec2(center.x - 7, center.y - 6), ImVec2(center.x + 7, center.y - 6), IM_COL32(100, 105, 115, 255), 1.5f);
            draw_list->AddLine(ImVec2(center.x - 7, center.y), ImVec2(center.x + 5, center.y), IM_COL32(100, 105, 115, 255), 1.5f);
            draw_list->AddLine(ImVec2(center.x - 7, center.y + 6), ImVec2(center.x + 2, center.y + 6), IM_COL32(100, 105, 115, 255), 1.5f);
        }
    }

    void AssetBrowser::render_header() {
        if (current_directory_ != root_directory_) {
            if (ImGui::Button("← Back")) {
                current_directory_ = current_directory_.parent_path();
            }
            ImGui::SameLine();
        }
        ImGui::Text("%s", current_directory_.string().c_str());
        ImGui::Separator();
    }

    void AssetBrowser::render_grid() {
        constexpr float padding = 16.0f;
        constexpr float thumbnail_size = 72.0f;
        constexpr float cell_size = thumbnail_size + padding;

        const float panel_width = ImGui::GetContentRegionAvail().x;
        int column_count = static_cast<int>(panel_width / cell_size);
        if (column_count < 1) column_count = 1;

        if (!std::filesystem::exists(current_directory_)) return;

        if (ImGui::BeginTable("AssetGrid", column_count, ImGuiTableFlags_SizingFixedSame)) {
            for (const auto& entry : std::filesystem::directory_iterator(current_directory_)) {
                if (entry.path().extension() == ".meta") continue;

                ImGui::TableNextColumn();
                render_asset_item(entry, thumbnail_size);
            }
            ImGui::EndTable();
        }
    }

    void AssetBrowser::render_asset_item(const std::filesystem::directory_entry& entry, const float thumbnail_size) {
        const auto& path = entry.path();
        const std::string filename = path.filename().string();
        const bool is_directory = entry.is_directory();

        ImGui::PushID(filename.c_str());

        // Button background colors
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        if (is_directory) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.22f, 0.26f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.30f, 0.36f, 1.00f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.16f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.25f, 1.00f));
        }
        // Base Button + Icon Overlay
        ImGui::Button("##thumb", ImVec2(thumbnail_size, thumbnail_size));
        draw_item_icon(ImGui::GetWindowDrawList(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), is_directory);

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        // Double-click folder navigation
        if (is_directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            current_directory_ /= path.filename();
        }
        // Drag & Drop payload source
        if (!is_directory && asset_manager_) {
            engine::AssetHandle handle = asset_manager_->get_handle_from_path(path);
            if (handle != 0 && ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &handle, sizeof(engine::AssetHandle));
                ImGui::Text("Assign %s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }
        // Filename Label
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnail_size);
        ImGui::TextUnformatted(filename.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopID();
    }
    #endif
}
