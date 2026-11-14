//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>

#include "asset/Asset.hpp"

namespace engine {
    class MetaFileHandler {
    public:
        /** @brief If the asset exists it returns it, generates a new one otherwise */
        static AssetMetadata get(const std::filesystem::path& asset_path);

    private:
        static Asset deduce_asset_type(const std::filesystem::path& filepath);
        static std::filesystem::path find_shader_source(const std::filesystem::path& compiled_path);
    };
}
