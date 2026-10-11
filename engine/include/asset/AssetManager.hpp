//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <unordered_map>

#include "AssetType.hpp"
#include "MetafileHandler.hpp"
#include "render/gpu/GPUData.hpp"
#include "render/mesh/Mesh.hpp"
#include "render/mesh/MeshLoader.hpp"

namespace engine {
    class GPUMeshManager;
    class MaterialSystem;
    class TextureSystem;

    class AssetManager {
    public:
        AssetManager() = default;
        AssetManager(GPUMeshManager& mesh_manager, MaterialSystem& material_system, TextureSystem& texture_system);
        ~AssetManager() = default;

        /** @brief Binds the GPU subsystem dependencies required to upload assets during loading. */
        void initialize_gpu_systems(GPUMeshManager& mesh_manager,
                                    MaterialSystem& material_system,
                                    TextureSystem& texture_system);

        /** @brief Scans a directory recursively, creating .meta files and registering asset handles. */
        void scan_directory(const std::filesystem::path& asset_directory);

        [[nodiscard]] const AssetMetadata& get_metadata(AssetHandle handle) const;
        [[nodiscard]] const AssetMetadata& get_metadata(const std::filesystem::path& filepath) const;

        [[nodiscard]] AssetHandle get_handle_from_path(const std::filesystem::path& filepath) const;
        [[nodiscard]] std::filesystem::path get_path_from_handle(AssetHandle handle) const;

        [[nodiscard]] bool is_asset_handle_valid(const AssetHandle handle) const { return asset_registry_.contains(handle); }

        [[nodiscard]] const std::unordered_map<AssetHandle, AssetMetadata>& get_registry() const { return asset_registry_; }

        /**
         * @brief Generic asset getter fetching from CPU cache or loading from disk into GPU mega-buffers.
         */
        template<typename T>
        std::shared_ptr<T> get_asset(AssetHandle handle);

        /**
         * @brief Specialized texture helper returning the global TextureArray slice index for GPUMaterialData.
         */
        uint32_t get_texture_index(AssetHandle handle);

        void clear_cache() {
            mesh_cache_.clear();
            texture_index_cache_.clear();
        }

    private:
        GPUMeshManager* mesh_manager_{nullptr};
        MaterialSystem* material_system_{nullptr};
        TextureSystem* texture_system_{nullptr};

        std::unordered_map<AssetHandle, AssetMetadata> asset_registry_;
        std::unordered_map<std::filesystem::path, AssetHandle> path_to_handle_map_;

        // Asset Caches
        std::unordered_map<AssetHandle, std::shared_ptr<Mesh>> mesh_cache_;
        std::unordered_map<AssetHandle, uint32_t> texture_index_cache_;

        inline static AssetMetadata null_metadata_{};
    };

    template<>
    inline std::shared_ptr<Mesh> AssetManager::get_asset<Mesh>(const AssetHandle handle) {
        auto it = mesh_cache_.find(handle);
        if (it != mesh_cache_.end()) {
            return it->second;
        }

        const auto& metadata = get_metadata(handle);
        if (!metadata.is_valid() || metadata.type != AssetType::MESH) {
            return nullptr;
        }

        if (!mesh_manager_ || !material_system_ || !texture_system_) {
            return nullptr; // GPU managers not initialized
        }

        Mesh loaded_mesh = MeshLoader::load(metadata.filepath.string(),
                                            *mesh_manager_,
                                            *material_system_,
                                            *texture_system_);

        auto mesh_ptr = std::make_shared<Mesh>(std::move(loaded_mesh));
        mesh_cache_[handle] = mesh_ptr;
        return mesh_ptr;
    }
}
