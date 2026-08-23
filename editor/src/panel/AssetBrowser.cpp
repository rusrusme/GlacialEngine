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

        // 1. Navigation / Pfad-Header
        if (current_directory_ != root_directory_) {
            if (ImGui::Button("<- Back")) {
                current_directory_ = current_directory_.parent_path();
            }
            ImGui::SameLine();
        }
        ImGui::Text("Directory: %s", current_directory_.string().c_str());
        ImGui::Separator();

        // 2. Grid-Layout für Dateien & Ordner
        constexpr float padding = 16.0f;
        constexpr float thumbnail_size = 80.0f;
        constexpr float cell_size = thumbnail_size + padding;

        const float panel_width = ImGui::GetContentRegionAvail().x;
        int column_count = static_cast<int>(panel_width / cell_size);
        if (column_count < 1) column_count = 1;

        ImGui::Columns(column_count, nullptr, false);

        if (std::filesystem::exists(current_directory_)) {
            for (const auto& directory_entry : std::filesystem::directory_iterator(current_directory_)) {
                const auto& path = directory_entry.path();
                const std::string filename = path.filename().string();

                // .meta Dateien im Browser ausblenden
                if (path.extension() == ".meta") continue;

                ImGui::PushID(filename.c_str());

                const bool is_directory = directory_entry.is_directory();
                const std::string button_label = is_directory ? "[DIR]\n" + filename : "[FILE]\n" + filename;

                ImGui::Button(button_label.c_str(), ImVec2(thumbnail_size, thumbnail_size));

                // Ordner per Doppelklick öffnen
                if (is_directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    current_directory_ /= path.filename();
                }

                // 3. Drag & Drop Source für Dateien (Handles versenden)
                if (!is_directory && asset_manager_) {
                    engine::AssetHandle handle = asset_manager_->get_handle_from_path(path);

                    if (handle != 0 && ImGui::BeginDragDropSource()) {
                        // Wir übergeben das AssetHandle als Drag-Payload
                        ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &handle, sizeof(engine::AssetHandle));
                        ImGui::Text("Assign %s", filename.c_str());
                        ImGui::EndDragDropSource();
                    }
                }

                ImGui::NextColumn();
                ImGui::PopID();
            }
        }

        ImGui::Columns(1);
        ImGui::End();
    }
    #endif
}
