//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <stb_image.h>
#include <unordered_map>

#include "asset/AssetType.hpp"
#include "core/Log.hpp"
#include "render/mesh/Mesh.hpp"
#include "render/mesh/MeshLoader.hpp"
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

    ////////////////////////////////////////////////////////////////////////////////////////////
    template<>
    inline std::shared_ptr<Mesh> AssetManager::get<Mesh>(const AssetHandle handle) {
        auto it = mesh_cache_.find(handle);
        if (it != mesh_cache_.end()) {
            return it->second;
        }

        const auto& meta = metadata(handle);
        if (!meta.valid() || meta.type != AssetType::MESH) {
            return nullptr;
        }

        // Primitives ("primitive://cube")
        if (meta.filepath.string() == "primitive://cube") {
            if (!device_) {
                CORE_ERROR("[AssetManager] Cannot create primitive cube: RHIDevice is not initialized!");
                return nullptr;
            }
            auto cube = Mesh::create_cube(*device_);
            mesh_cache_[handle] = cube;
            return cube;
        }

        auto loaded_mesh = MeshLoader::load(*device_, meta.filepath);
        if (!loaded_mesh) {
            CORE_ERROR("[AssetManager] Failed to load mesh from '{0}'", meta.filepath.string());
            return nullptr;
        }

        mesh_cache_[handle] = loaded_mesh;
        return loaded_mesh;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    template<>
    inline std::shared_ptr<Texture2D> AssetManager::get<Texture2D>(const AssetHandle handle) {
        auto it = texture_cache_.find(handle);
        if (it != texture_cache_.end()) {
            return it->second;
        }

        const auto& meta = metadata(handle);
        if (!meta.valid() || meta.type != AssetType::TEXTURE_2D) {
            return nullptr;
        }

        if (!device_) {
            CORE_ERROR("[AssetManager] Cannot create Texture2D: RHIDevice not initialized!");
            return nullptr;
        }

        int width = 0, height = 0, channels = 0;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(meta.filepath.string().c_str(), &width, &height, &channels, 4);

        if (!data) {
            CORE_ERROR("[AssetManager] Failed to load image file: '{0}'", meta.filepath.string());
            return nullptr;
        }

        TextureSpecification spec{};
        spec.width = static_cast<uint32_t>(width);
        spec.height = static_cast<uint32_t>(height);
        spec.format = TextureFormat::RGBA8;
        spec.generate_mipmaps = true;

        auto texture = device_->create_texture_2d(spec, data);

        stbi_image_free(data);

        if (!texture || texture->renderer_id() == 0) {
            CORE_ERROR("[AssetManager] Failed to allocate GPU texture for '{0}'", meta.filepath.string());
            return nullptr;
        }

        std::shared_ptr shared_texture = std::move(texture);
        texture_cache_[handle] = shared_texture;
        return shared_texture;
    }
}
