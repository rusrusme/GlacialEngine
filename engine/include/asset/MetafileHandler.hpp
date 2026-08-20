//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>

#include "AssetType.hpp"

namespace engine {
    class MetaFileHandler {
    public:
        /** @brief If the asset exists it returns it, generates a new one otherwise */
        static AssetMetadata get_or_create_metadata(const std::filesystem::path& asset_path);

    private:
        static AssetType deduce_asset_type(const std::filesystem::path& filepath);
    };
}
