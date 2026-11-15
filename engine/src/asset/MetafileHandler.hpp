//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include "asset/Asset.hpp"

namespace engine {
    class MetafileHandler {
    public:
        static void register_defaults();
        static void register_extension(std::string_view extension, AssetType type, std::string_view type_name);

        /** @brief Loads existing metadata or creates a new .meta file if generate_missing is true. */
        static AssetMetadata resolve(const std::filesystem::path& asset_path, bool generate_missing = true);

    private:
        struct ExtensionEntry {
            AssetType type;
            std::string name;
        };

        static const ExtensionEntry* find_extension(const std::filesystem::path& path);
        static std::filesystem::path find_shader_source(const std::filesystem::path& compiled_path);

        inline static std::unordered_map<std::string, ExtensionEntry> extensions_{};
        inline static bool defaults_registered{false};
    };
}
