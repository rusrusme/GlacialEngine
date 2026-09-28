//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "asset/AssetManager.hpp"

namespace editor {
    class AssetBrowser {
    public:
        AssetBrowser() = default;

        void init(engine::SystemContext& context);
        void save_config();

        #ifdef ENABLE_IMGUI
        void render_imgui(engine::SystemContext& context);
        #endif

    private:
        #ifdef ENABLE_IMGUI
        void render_header();
        void render_assets_view(engine::SystemContext& context);
        void render_grid_view(const std::filesystem::directory_entry& entry, engine::SystemContext& context);
        void render_list_view(const std::filesystem::directory_entry& entry, engine::SystemContext& context);
        #endif

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
