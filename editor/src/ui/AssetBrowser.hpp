//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <asset/AssetManager.hpp>

namespace editor {
    class AssetBrowser {
    public:
        AssetBrowser() = default;

        void init(std::shared_ptr<engine::AssetManager> asset_manager);
        void render_ui();

    private:
        std::shared_ptr<engine::AssetManager> asset_manager_ = nullptr;
        std::filesystem::path root_directory_{"assets"};
        std::filesystem::path current_directory_{root_directory_};
    };
}
