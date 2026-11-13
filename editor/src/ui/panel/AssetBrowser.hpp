//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>

#include "project/Project.hpp"
#include "ui/UIComponent.hpp"

namespace editor {
    using namespace engine;

    class AssetBrowser : public UIComponent {
    public:
        AssetBrowser() : UIComponent("Asset Browser") {}
        ~AssetBrowser() override = default;

        void on_init(SystemContext& context) override;
        void on_update(SystemContext& context, float delta_time) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;
        bool sync_project_directory(SystemContext& context);

        [[nodiscard]] ShortcutContext shortcut_context() const override { return ShortcutContext::AssetBrowser; }
        
    private:
        void render_header();
        void render_assets_view(SystemContext& context);
        void render_grid_view(const std::filesystem::directory_entry& entry, SystemContext& context);
        void render_list_view(const std::filesystem::directory_entry& entry, SystemContext& context);

        std::shared_ptr<Project> active_project_ = nullptr;

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
