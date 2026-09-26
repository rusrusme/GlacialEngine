//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "asset/AssetManager.hpp"

namespace editor {
    class AssetBrowser {
    public:
        AssetBrowser() = default;

        void init(std::shared_ptr<engine::AssetManager> asset_manager);
        void save_config();

        #ifdef ENABLE_IMGUI
        void render_imgui();
        #endif

    private:
        #ifdef ENABLE_IMGUI
        void render_header();
        void render_assets_view();
        void render_grid_view(const std::filesystem::directory_entry& entry);
        void render_list_view(const std::filesystem::directory_entry& entry);
        #endif

        std::shared_ptr<engine::AssetManager> asset_manager_ = nullptr;
        std::filesystem::path root_directory_{"assets"};
        std::filesystem::path current_directory_{root_directory_};
        std::optional<std::filesystem::path> pending_directory_change_ = std::nullopt;

        bool sort_ascending_ = true;

        float thumbnail_size_ = 64.0f;
        float min_thumbnail_size = 32.0f;
        float max_thumbnail_size = 96.0f;

        char search_filter_[128] = "";
    };
}
