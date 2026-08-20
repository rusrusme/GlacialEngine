//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <asset/AssetManager.hpp>

#include <iostream>
#include <asset/MetafileHandler.hpp>

namespace engine {
    void AssetManager::scan_directory(const std::filesystem::path& asset_directory) {
        asset_registry_.clear();
        path_to_handle_map_.clear();

        auto register_primitive = [this](const std::string& path, uint64_t fixed_id) {
            AssetHandle handle(fixed_id);
            AssetMetadata meta{
                .handle = handle,
                .type = AssetType::MESH,
                .filepath = path
            };
            asset_registry_[handle] = meta;
            path_to_handle_map_[path] = handle;
        };

        register_primitive("primitive://plane",    1);
        register_primitive("primitive://circle",   2);
        register_primitive("primitive://cube",     3);
        register_primitive("primitive://sphere",   4);
        register_primitive("primitive://cylinder", 5);

        if (!std::filesystem::exists(asset_directory) || !std::filesystem::is_directory(asset_directory)) {
            return;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(asset_directory)) {
            if (!entry.is_regular_file()) continue;
            const auto& path = entry.path();
            if (path.extension() == ".meta") continue;

            AssetMetadata metadata = MetaFileHandler::get_or_create_metadata(path);
            if (metadata.is_valid()) {
                asset_registry_[metadata.handle] = metadata;
                path_to_handle_map_[metadata.filepath] = metadata.handle;
            }
        }
    }

    const AssetMetadata& AssetManager::get_metadata(AssetHandle handle) const {
        auto it = asset_registry_.find(handle);
        if (it != asset_registry_.end()) {
            return it->second;
        }
        return null_metadata_;
    }

    const AssetMetadata& AssetManager::get_metadata(const std::filesystem::path& filepath) const {
        AssetHandle handle = get_handle_from_path(filepath);
        return get_metadata(handle);
    }

    AssetHandle AssetManager::get_handle_from_path(const std::filesystem::path& filepath) const {
        auto it = path_to_handle_map_.find(filepath);
        if (it != path_to_handle_map_.end()) {
            return it->second;
        }
        return {0};
    }
}
