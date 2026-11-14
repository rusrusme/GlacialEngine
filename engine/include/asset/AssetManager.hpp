//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <unordered_map>

#include "asset/AssetType.hpp"
#include "core/Log.hpp"
#include "render/Mesh.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    class AssetManager {
    public:
        AssetManager() = default;
        ~AssetManager() = default;

        void init(RHIDevice& device) { device_ = &device; }

        void scan_directory(const std::filesystem::path& asset_directory);

        [[nodiscard]] const AssetMetadata& metadata(AssetHandle handle) const;
        [[nodiscard]] const AssetMetadata& metadata(const std::filesystem::path& filepath) const;

        [[nodiscard]] AssetHandle asset_handle_from_path(const std::filesystem::path& filepath) const;
        [[nodiscard]] std::filesystem::path path_from_asset_handle(AssetHandle handle) const;
        [[nodiscard]] std::filesystem::path source_path_from_asset_handle(AssetHandle handle) const;
        [[nodiscard]] bool valid_asset(const AssetHandle handle) const { return asset_registry_.contains(handle); }

        [[nodiscard]] const std::unordered_map<AssetHandle, AssetMetadata>& registry() const { return asset_registry_; }

        template<typename T>
        std::shared_ptr<T> get(AssetHandle handle);

        void clear_cache() {
            mesh_cache_.clear();
            texture_cache_.clear();
        }

    private:
        RHIDevice* device_{nullptr};

        std::unordered_map<AssetHandle, AssetMetadata> asset_registry_;
        std::unordered_map<std::filesystem::path, AssetHandle> path_to_handle_map_;

        // Asset Caches
        std::unordered_map<AssetHandle, std::shared_ptr<Mesh>> mesh_cache_;
        std::unordered_map<AssetHandle, std::shared_ptr<Texture2D>> texture_cache_;

        inline static AssetMetadata null_metadata_{};
    };

    template<>
    std::shared_ptr<Mesh> AssetManager::get<Mesh>(AssetHandle handle);

    template<>
    std::shared_ptr<Texture2D> AssetManager::get<Texture2D>(AssetHandle handle);
}
