//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <unordered_map>
#include <render/mesh/Mesh.hpp>
#include <render/mesh/MeshLoader.hpp>

#include "AssetType.hpp"

namespace engine {
    class AssetManager {
    public:
        AssetManager() = default;

        void scan_directory(const std::filesystem::path& asset_directory);

        [[nodiscard]] const AssetMetadata& get_metadata(AssetHandle handle) const;
        [[nodiscard]] const AssetMetadata& get_metadata(const std::filesystem::path& filepath) const;
        [[nodiscard]] AssetHandle get_handle_from_path(const std::filesystem::path& filepath) const;

        [[nodiscard]] bool is_asset_handle_valid(AssetHandle handle) const {
            return asset_registry_.contains(handle);
        }

        [[nodiscard]] const std::unordered_map<AssetHandle, AssetMetadata>& get_registry() const {
            return asset_registry_;
        }

        template <typename T>
        std::shared_ptr<T> get_asset(AssetHandle handle);

        void clear_cache() {
            mesh_cache_.clear();
        }

    private:
        std::unordered_map<AssetHandle, AssetMetadata> asset_registry_;
        std::unordered_map<std::filesystem::path, AssetHandle> path_to_handle_map_;

        std::unordered_map<AssetHandle, std::shared_ptr<Mesh>> mesh_cache_;

        inline static AssetMetadata null_metadata_{};
    };

    template <typename T>
    std::shared_ptr<T> AssetManager::get_asset(const AssetHandle handle) {
        // 1. Prüfen, ob das Mesh bereits geladen und im Cache ist
        auto it = mesh_cache_.find(handle);
        if (it != mesh_cache_.end()) {
            return it->second;
        }

        // 2. Metadaten abfragen
        const auto& metadata = get_metadata(handle);
        if (!metadata.is_valid() || metadata.type != AssetType::MESH) {
            return nullptr;
        }

        // 3. Wenn nicht im Cache -> Über MeshLoader von Festplatte laden
        Mesh loaded_mesh = MeshLoader::load(metadata.filepath.string());
        if (!loaded_mesh.is_valid()) {
            return nullptr;
        }

        // 4. Im Cache ablegen und als shared_ptr zurückgeben
        auto mesh_ptr = std::make_shared<Mesh>(std::move(loaded_mesh));
        mesh_cache_[handle] = mesh_ptr;
        return mesh_ptr;
    }
}
