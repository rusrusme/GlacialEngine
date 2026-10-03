//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "AssetBrowser.hpp"

#include <algorithm>
#include <vector>

#include "project/Project.hpp"

#include "Config.hpp"
#include "gui/Icons.hpp"

namespace editor {
    void AssetBrowser::init(engine::SystemContext& context) {
        const float thumbnail_size = config::get_value("asset_thumbnail_size", thumbnail_size_);
        thumbnail_size_ = thumbnail_size;

        const bool sort_order = config::get_value("asset_sort_ascending", sort_ascending_);
        sort_ascending_ = sort_order;
    }

    void AssetBrowser::save_config() {
        config::set_value("asset_thumbnail_size", thumbnail_size_);
        config::set_value("asset_sort_ascending", sort_ascending_);
    }

    #ifdef ENABLE_IMGUI
    void AssetBrowser::render_imgui(engine::SystemContext& context) {
        update_project(context);

        ImGui::Begin("Asset Browser");
        render_header();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        if (ImGui::BeginChild("AssetGridScrollArea", ImVec2(0, 0), false)) {
            render_assets_view(context);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::End();
    }

    static void draw_item_icon(ImDrawList* draw_list, const ImVec2 p0, const ImVec2 p1, const bool is_directory) {
        const auto center = ImVec2((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
        const float w = p1.x - p0.x;
        const float h = p1.y - p0.y;
        const float scale = std::clamp(std::min(w, h) / 52.0f, 0.35f, 2.0f);

        if (is_directory) {
            constexpr ImU32 FOLDER_COLOR = IM_COL32(210, 170, 90, 220);
            draw_list->AddRectFilled(
                ImVec2(center.x - 14.0f * scale, center.y - 11.0f * scale),
                ImVec2(center.x - 3.0f * scale, center.y - 8.0f * scale),
                FOLDER_COLOR, 2.0f * scale
            );
            draw_list->AddRectFilled(
                ImVec2(center.x - 14.0f * scale, center.y - 8.0f * scale),
                ImVec2(center.x + 14.0f * scale, center.y + 9.0f * scale),
                FOLDER_COLOR, 2.0f * scale
            );
        }
        else {
            constexpr ImU32 FILE_COLOR = IM_COL32(180, 185, 195, 200);
            draw_list->AddRectFilled(
                ImVec2(center.x - 9.0f * scale, center.y - 12.0f * scale),
                ImVec2(center.x + 9.0f * scale, center.y + 12.0f * scale),
                FILE_COLOR, 2.0f * scale
            );
            draw_list->AddLine(ImVec2(center.x - 5.0f * scale, center.y - 5.0f * scale), ImVec2(center.x + 5.0f * scale, center.y - 5.0f * scale), IM_COL32(100, 105, 115, 255), 1.2f * scale);
            draw_list->AddLine(ImVec2(center.x - 5.0f * scale, center.y), ImVec2(center.x + 3.0f * scale, center.y), IM_COL32(100, 105, 115, 255), 1.2f * scale);
            draw_list->AddLine(ImVec2(center.x - 5.0f * scale, center.y + 5.0f * scale), ImVec2(center.x + 1.0f * scale, center.y + 5.0f * scale), IM_COL32(100, 105, 115, 255), 1.2f * scale);
        }
    }

    void AssetBrowser::render_header() {
        // --- Row 1 --- Breadcrumbs Path ---
        std::filesystem::path accumulated_path = root_directory_;
        std::optional<std::filesystem::path> new_directory = std::nullopt;

        // Get path relative to the asset root
        std::filesystem::path relative_path = std::filesystem::relative(current_directory_, root_directory_);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        // Root button ("assets")
        if (ImGui::Button("assets")) {
            new_directory = root_directory_;
        }

        // Render subfolders relative to root
        if (relative_path != ".") {
            for (const auto& part : relative_path) {
                ImGui::SameLine();
                ImGui::TextDisabled("/");
                ImGui::SameLine();

                accumulated_path /= part;
                const bool is_current = (accumulated_path == current_directory_);

                ImGui::PushID(accumulated_path.string().c_str());
                ImGui::PushStyleColor(ImGuiCol_Text, is_current ? ImVec4(0.95f, 0.95f, 0.95f, 1.00f) : ImVec4(0.60f, 0.63f, 0.68f, 1.00f));

                if (ImGui::Button(part.string().c_str())) {
                    new_directory = accumulated_path;
                }

                ImGui::PopStyleColor();
                ImGui::PopID();
            }
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);

        if (new_directory.has_value()) {
            current_directory_ = *new_directory;
        }

        // Margin between rows
        ImGui::Dummy(ImVec2(0.0f, 2.0f));

        // --- Row 2 --- Toolbar Controls ---
        const float avail_x = ImGui::GetContentRegionAvail().x;
        constexpr float slider_width = 64.0f;
        constexpr float sort_btn_width = 28.0f;
        constexpr float spacing = 4.0f;
        constexpr float right_controls_width = sort_btn_width + spacing + slider_width;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 16.0f);

        // Search Bar
        const float search_width = std::max(80.0f, avail_x - right_controls_width - spacing);

        ImGui::PushItemWidth(search_width);
        if (search_filter_[0] != '\0') {
            ImGui::SetNextItemAllowOverlap(); // Allow clear button overlay to capture clicks
        }
        ImGui::InputTextWithHint("##search_filter", "Search Assets", search_filter_, sizeof(search_filter_));
        ImGui::PopItemWidth();

        const ImVec2 search_min = ImGui::GetItemRectMin();
        const ImVec2 search_max = ImGui::GetItemRectMax();

        // Direct SameLine chain maintains deterministic vertical alignment
        ImGui::SameLine(0.0f, spacing);

        const float current_x = ImGui::GetCursorPosX();
        const float desired_x = avail_x - right_controls_width;
        if (desired_x > current_x) {
            ImGui::SetCursorPosX(desired_x);
        }

        // Sort Direction Toggle Button
        if (ImGui::Button("##sort_dir", ImVec2(sort_btn_width, 22.0f))) {
            sort_ascending_ = !sort_ascending_;
        }
        const ImVec2 btn_min = ImGui::GetItemRectMin();
        const ImVec2 btn_size = ImGui::GetItemRectSize();
        const ImVec2 icon_pos(
            btn_min.x + (btn_size.x - 14.0f) * 0.5f,
            btn_min.y + (btn_size.y - 14.0f) * 0.5f
        );

        const ImU32 sort_icon_col = ImGui::IsItemHovered()
                                        ? IM_COL32(255, 255, 255, 255)
                                        : IM_COL32(180, 185, 195, 220);

        draw_sort_icon(ImGui::GetWindowDrawList(), icon_pos, sort_ascending_, sort_icon_col);

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Sort %s", sort_ascending_ ? "Ascending" : "Descending");
        }

        ImGui::SameLine(0.0f, spacing);

        // View Size Slider
        ImGui::PushItemWidth(slider_width);
        ImGui::SliderFloat("##thumbnail_size", &thumbnail_size_, min_thumbnail_size, max_thumbnail_size, "");

        const bool is_list_view = (thumbnail_size_ <= min_thumbnail_size);

        if (is_list_view) {
            const ImVec2 slider_min = ImGui::GetItemRectMin();
            const ImVec2 slider_max = ImGui::GetItemRectMax();
            const float grab_w = std::max(ImGui::GetStyle().GrabMinSize, 16.0f);

            const ImVec2 grab_center(
                std::floor(slider_min.x + 2.0f + grab_w * 0.5f),
                std::floor((slider_min.y + slider_max.y) * 0.5f)
            );

            const ImU32 list_icon_col = (ImGui::IsItemHovered() || ImGui::IsItemActive())
                                            ? IM_COL32(255, 255, 255, 255)
                                            : IM_COL32(180, 185, 195, 220);

            draw_list_icon(ImGui::GetWindowDrawList(), grab_center, list_icon_col);
        }

        if (ImGui::IsItemHovered()) {
            if (is_list_view) {
                ImGui::SetTooltip("List View");
            }
            else {
                ImGui::SetTooltip("Size %.0f", thumbnail_size_);
            }
        }

        ImGui::PopItemWidth();

        const ImVec2 post_row_cursor = ImGui::GetCursorScreenPos();
        // --- Overlay Pass --- Embedded Clear Button ---
        if (search_filter_[0] != '\0') {
            const float clear_btn_size = search_max.y - search_min.y - 4.0f;
            const ImVec2 clear_btn_pos(search_max.x - clear_btn_size - 3.0f, search_min.y + 2.0f);

            ImGui::SetCursorScreenPos(clear_btn_pos);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.15f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.25f));

            if (ImGui::Button("##clear_search", ImVec2(clear_btn_size, clear_btn_size))) {
                search_filter_[0] = '\0';
            }

            const ImVec2 cb_min = ImGui::GetItemRectMin();
            const ImVec2 cb_max = ImGui::GetItemRectMax();
            const ImVec2 cb_center(
                std::floor((cb_min.x + cb_max.x) * 0.5f),
                std::floor((cb_min.y + cb_max.y) * 0.5f)
            );

            const ImU32 x_col = ImGui::IsItemHovered()
                                    ? IM_COL32(255, 255, 255, 255)
                                    : IM_COL32(160, 165, 175, 200);

            constexpr float cross_r = 3.0f;
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddLine(ImVec2(cb_center.x - cross_r, cb_center.y - cross_r), ImVec2(cb_center.x + cross_r, cb_center.y + cross_r), x_col, 1.2f);
            draw_list->AddLine(ImVec2(cb_center.x + cross_r, cb_center.y - cross_r), ImVec2(cb_center.x - cross_r, cb_center.y + cross_r), x_col, 1.2f);

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(2);
        }

        ImGui::SetCursorScreenPos(post_row_cursor);
        ImGui::PopStyleVar(2);
        ImGui::Separator();
    }

    void AssetBrowser::render_assets_view(engine::SystemContext& context) {
        if (!std::filesystem::exists(current_directory_)) return;

        // Prepare lower-case search filter string
        std::string filter_query = search_filter_;
        std::ranges::transform(filter_query, filter_query.begin(), [](const unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        const bool is_searching = !filter_query.empty();
        std::vector<std::filesystem::directory_entry> entries;

        auto process_entry = [&](const std::filesystem::directory_entry& entry) {
            if (entry.path().extension() == ".meta") return;

            if (is_searching) {
                std::string filename = entry.path().filename().string();
                std::ranges::transform(filename, filename.begin(), [](const unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });

                if (filename.find(filter_query) == std::string::npos) {
                    return; // Skip non-matching items
                }
            }

            entries.push_back(entry);
        };

        if (is_searching) {
            // Recursive traversal into all child folders
            std::error_code ec;
            constexpr auto options = std::filesystem::directory_options::skip_permission_denied;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(current_directory_, options, ec)) {
                process_entry(entry);
            }
        }
        else {
            // Single-level traversal for standard browsing
            for (const auto& entry : std::filesystem::directory_iterator(current_directory_)) {
                process_entry(entry);
            }
        }

        std::ranges::sort(entries, [this](const auto& a, const auto& b) {
            const bool a_is_dir = a.is_directory();
            const bool b_is_dir = b.is_directory();

            // Folders always appear at top
            if (a_is_dir != b_is_dir) {
                return a_is_dir > b_is_dir;
            }

            const std::string name_a = a.path().filename().string();
            const std::string name_b = b.path().filename().string();

            // Convert strings to lowercase for case-insensitive ordering
            std::string lower_a = name_a;
            std::string lower_b = name_b;
            std::ranges::transform(lower_a, lower_a.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            std::ranges::transform(lower_b, lower_b.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (lower_a != lower_b) {
                return sort_ascending_ ? (lower_a < lower_b) : (lower_a > lower_b);
            }

            // Tie-breaker fallback if names only differ by case (e.g., "Hi" vs "hi")
            return sort_ascending_ ? (name_a < name_b) : (name_a > name_b);
        });

        const bool is_list_view = (thumbnail_size_ <= min_thumbnail_size);

        if (is_list_view) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));
            for (const auto& entry : entries) {
                render_list_view(entry, context);
            }
            ImGui::PopStyleVar();
        }
        else {
            const float cell_size = thumbnail_size_ + 10.0f;
            const float panel_width = ImGui::GetContentRegionAvail().x;
            int column_count = static_cast<int>(panel_width / cell_size);
            if (column_count < 1) column_count = 1;

            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));

            if (ImGui::BeginTable("AssetGrid", column_count, ImGuiTableFlags_SizingStretchSame)) {
                for (const auto& entry : entries) {
                    ImGui::TableNextColumn();
                    render_grid_view(entry, context);
                }
                ImGui::EndTable();
            }

            ImGui::PopStyleVar(2);
        }

        // Apply deferred navigation safely outside loop
        if (pending_directory_change_.has_value()) {
            current_directory_ = *pending_directory_change_;
            pending_directory_change_ = std::nullopt;
        }
    }

    void AssetBrowser::render_grid_view(const std::filesystem::directory_entry& entry, engine::SystemContext& context) {
        const auto& path = entry.path();
        const std::string filename = path.filename().string();
        const bool is_directory = entry.is_directory();

        ImGui::PushID(filename.c_str());

        const float column_width = ImGui::GetColumnWidth();
        const float x_offset = std::max(0.0f, (column_width - thumbnail_size_) * 0.5f);
        const float start_x = ImGui::GetCursorPosX() + x_offset;
        ImGui::SetCursorPosX(start_x);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        if (is_directory) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.22f, 0.26f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.30f, 0.36f, 1.00f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.16f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.25f, 1.00f));
        }

        ImGui::Button("##thumb", ImVec2(thumbnail_size_, thumbnail_size_));
        draw_item_icon(ImGui::GetWindowDrawList(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), is_directory);

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        if (is_directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            pending_directory_change_ = path;
        }

        if (!is_directory) {
            engine::AssetHandle handle = context.assets.get_handle_from_path(path);
            if (handle != 0 && ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &handle, sizeof(engine::AssetHandle));
                ImGui::Text("Assign %s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }

        if (search_filter_[0] != '\0' && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", std::filesystem::relative(path, root_directory_).string().c_str());
        }

        ImGui::SetCursorPosX(start_x);
        ImGui::PushTextWrapPos(start_x + thumbnail_size_);
        ImGui::TextUnformatted(filename.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopID();
    }

    void AssetBrowser::render_list_view(const std::filesystem::directory_entry& entry, engine::SystemContext& context) {
        const auto& path = entry.path();
        const std::string filename = path.filename().string();
        const bool is_directory = entry.is_directory();

        ImGui::PushID(filename.c_str());

        const float row_height = 24.0f;
        const float avail_x = ImGui::GetContentRegionAvail().x;
        const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

        // Use Selectable for clean row selection & automatic line advancing
        ImGui::Selectable("##row", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(avail_x, row_height));
        const bool is_row_hovered = ImGui::IsItemHovered();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 p0 = cursor_pos;
        const ImVec2 p1 = ImVec2(p0.x + avail_x, p0.y + row_height);

        // Render Icon on the left
        draw_item_icon(draw_list, ImVec2(p0.x + 4.0f, p0.y + 2.0f), ImVec2(p0.x + 22.0f, p1.y - 2.0f), is_directory);

        // Standard text color for both folders and files
        const ImU32 text_color = ImGui::GetColorU32(ImGuiCol_Text);

        draw_list->AddText(
            ImVec2(p0.x + 28.0f, p0.y + (row_height - ImGui::GetTextLineHeight()) * 0.5f),
            text_color,
            filename.c_str()
        );

        if (is_directory && is_row_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            pending_directory_change_ = path;
        }

        if (!is_directory) {
            engine::AssetHandle handle = context.assets.get_handle_from_path(path);
            if (handle != 0 && is_row_hovered && ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &handle, sizeof(engine::AssetHandle));
                ImGui::Text("Assign %s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }

        if (search_filter_[0] != '\0' && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", std::filesystem::relative(path, root_directory_).string().c_str());
        }

        ImGui::PopID();
    }

    void AssetBrowser::update_project(const engine::SystemContext& context) {
        if (context.current_project != active_project_) {
            active_project_ = context.current_project;
            if (active_project_) {
                root_directory_ = active_project_->resolve_virtual_path("assets://");
            }
            else {
                // Default fallback
                root_directory_ = "assets";
            }
            current_directory_ = root_directory_;
        }
    }
    #endif
}
