//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <asset/AssetManager.hpp>

#include "MetafileHandler.hpp"
#include "core/Log.hpp"
#include "render/mesh/MeshLoader.hpp"

namespace engine {
    void AssetManager::scan_directory(const std::filesystem::path& asset_directory) {
        CORE_INFO("Scanning asset directory: '{0}'", asset_directory.string());

        asset_registry_.clear();
        path_to_handle_map_.clear();

        auto register_primitive = [this](const std::string& path, uint64_t fixed_id) {
            AssetHandle handle(fixed_id);
            AssetMetadata meta{
                .handle   = handle,
                .type     = AssetType::MESH,
                .filepath = path
            };
            asset_registry_[handle] = meta;
            path_to_handle_map_[path] = handle;
        };

        register_primitive("primitive://plane", 1);
        register_primitive("primitive://circle", 2);
        register_primitive("primitive://cube", 3);
        register_primitive("primitive://sphere", 4);
        register_primitive("primitive://cylinder", 5);
        register_primitive("primitive://pyramid", 6);

        if (!std::filesystem::exists(asset_directory) || !std::filesystem::is_directory(asset_directory)) {
            CORE_WARN("Asset scan aborted: Directory '{0}' does not exist or is invalid", asset_directory.string());
            return;
        }

        size_t loaded_count = 0;
        size_t skipped_count = 0;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(asset_directory)) {
            if (!entry.is_regular_file()) continue;
            const auto& path = entry.path();
            if (path.extension() == ".meta") continue;

            AssetMetadata metadata = MetaFileHandler::get(path);
            if (metadata.valid()) {
                if (asset_registry_.contains(metadata.handle)) {
                    CORE_WARN("Asset handle collision! Handle {0} for '{1}' conflicts with existing asset",
                              static_cast<uint64_t>(metadata.handle), path.string());
                }

                asset_registry_[metadata.handle] = metadata;
                path_to_handle_map_[metadata.filepath] = metadata.handle;
                loaded_count++;

                CORE_TRACE("Registered asset: '{0}' [Handle: {1}]", path.string(), static_cast<uint64_t>(metadata.handle));
            }
            else {
                CORE_WARN("Skipped asset '{0}': Failed to deduce type or parse metadata", path.string());
                skipped_count++;
            }
        }
        CORE_INFO("Asset scan complete: {0} user assets loaded ({1} skipped), {2} total registered, including primitives",
                  loaded_count, skipped_count, asset_registry_.size());
    }

    const AssetMetadata& AssetManager::metadata(AssetHandle handle) const {
        auto it = asset_registry_.find(handle);
        if (it != asset_registry_.end()) {
            return it->second;
        }
        return null_metadata_;
    }

    const AssetMetadata& AssetManager::metadata(const std::filesystem::path& filepath) const {
        AssetHandle handle = asset_handle_from_path(filepath);
        return metadata(handle);
    }

    AssetHandle AssetManager::asset_handle_from_path(const std::filesystem::path& filepath) const {
        auto it = path_to_handle_map_.find(filepath);
        if (it != path_to_handle_map_.end()) {
            return it->second;
        }
        return {0};
    }

    std::filesystem::path AssetManager::path_from_asset_handle(AssetHandle handle) const {
        const auto& meta = metadata(handle);
        return meta.valid() ? meta.filepath : std::filesystem::path{};
    }

    std::filesystem::path AssetManager::source_path_from_asset_handle(AssetHandle handle) const {
        const auto& meta = metadata(handle);
        return meta.valid() ? meta.source_filepath : std::filesystem::path{};
    }

    template<>
    std::shared_ptr<Mesh> AssetManager::get<Mesh>(const AssetHandle handle) {
        auto it = mesh_cache_.find(handle);
        if (it != mesh_cache_.end()) {
            return it->second;
        }

        const auto& meta = metadata(handle);
        if (!meta.valid() || meta.type != AssetType::MESH) {
            return nullptr;
        }

        const std::string path_str = meta.filepath.string();

        // Primitives
        if (path_str.starts_with("primitive://")) {
            if (!device_) {
                CORE_ERROR("[AssetManager] Cannot create primitive '{0}': RHIDevice is not initialized!", path_str);
                return nullptr;
            }

            std::shared_ptr<Mesh> primitive = nullptr;
            if (path_str == "primitive://cube") {
                primitive = Mesh::create_cube(*device_);
            }
            else if (path_str == "primitive://plane") {
                primitive = Mesh::create_plane(*device_);
            }
            else if (path_str == "primitive://circle") {
                primitive = Mesh::create_circle(*device_);
            }
            else if (path_str == "primitive://sphere") {
                primitive = Mesh::create_sphere(*device_);
            }
            else if (path_str == "primitive://cylinder") {
                primitive = Mesh::create_cylinder(*device_);
            }
            else if (path_str == "primitive://pyramid") {
                primitive = Mesh::create_pyramid(*device_);
            }
            else {
                CORE_ERROR("[AssetManager] Unknown primitive type: '{0}'", path_str);
                return nullptr;
            }

            if (primitive) {
                mesh_cache_[handle] = primitive;
            }
            return primitive;
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
    std::shared_ptr<Texture2D> AssetManager::get<Texture2D>(const AssetHandle handle) {
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
