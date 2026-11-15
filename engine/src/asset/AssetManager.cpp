//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "asset/AssetManager.hpp"

#include <stb_image.h>

#include "asset/MetafileHandler.hpp"
#include "core/Log.hpp"
#include "render/Mesh.hpp"
#include "render/mesh/MeshLoader.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    void AssetManager::init(RHIDevice& device) {
        device_ = &device;
        MetafileHandler::register_defaults();
        register_primitives();

        register_loader<Mesh>([](const AssetManager& assets, const AssetMetadata& meta) -> std::shared_ptr<Mesh> {
            if (meta.type != Asset::MESH) return nullptr;

            RHIDevice* dev = assets.device();
            if (!dev) {
                CORE_ERROR("[AssetManager] Cannot load Mesh: RHIDevice is not initialized!");
                return nullptr;
            }

            const std::string path_str = meta.filepath.generic_string();
            if (path_str.starts_with("primitive://")) {
                if (path_str == "primitive://cube") return Mesh::create_cube(*dev);
                if (path_str == "primitive://plane") return Mesh::create_plane(*dev);
                if (path_str == "primitive://circle") return Mesh::create_circle(*dev);
                if (path_str == "primitive://sphere") return Mesh::create_sphere(*dev);
                if (path_str == "primitive://cylinder") return Mesh::create_cylinder(*dev);
                if (path_str == "primitive://pyramid") return Mesh::create_pyramid(*dev);

                CORE_ERROR("[AssetManager] Unknown primitive type: '{0}'", path_str);
                return nullptr;
            }

            auto loaded_mesh = MeshLoader::load(*dev, meta.filepath);
            if (!loaded_mesh) {
                CORE_ERROR("[AssetManager] Failed to load mesh from '{0}'", meta.filepath.string());
            }
            return loaded_mesh;
        });

        register_loader<Texture2D>([](const AssetManager& assets, const AssetMetadata& meta) -> std::shared_ptr<Texture2D> {
            if (meta.type != Asset::TEXTURE_2D) return nullptr;

            RHIDevice* dev = assets.device();
            if (!dev) {
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

            auto texture = dev->create_texture_2d(spec, data);
            stbi_image_free(data);

            if (!texture || texture->renderer_id() == 0) {
                CORE_ERROR("[AssetManager] Failed to allocate GPU texture for '{0}'", meta.filepath.string());
                return nullptr;
            }

            return texture;
        });
    }

    void AssetManager::scan_directory(const std::filesystem::path& asset_directory, bool generate_meta) {
        CORE_INFO("Scanning asset directory: '{0}'", asset_directory.string());

        if (!std::filesystem::exists(asset_directory) || !std::filesystem::is_directory(asset_directory)) {
            CORE_WARN("Asset scan aborted: Directory '{0}' does not exist or is invalid", asset_directory.string());
            return;
        }

        size_t loaded_count = 0;
        size_t skipped_count = 0;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(asset_directory)) {
            if (!entry.is_regular_file()) continue;
            const auto& file_path = entry.path();
            if (!MetafileHandler::is_asset_file(file_path)) continue;

            AssetMetadata metadata = MetafileHandler::resolve(file_path, generate_meta);
            if (metadata.valid()) {
                if (auto it = asset_registry_.find(metadata.handle);
                    it != asset_registry_.end() && it->second.filepath != metadata.filepath) {
                    CORE_WARN("Asset handle collision! Handle {0} for '{1}' conflicts with '{2}'",
                              static_cast<uint64_t>(metadata.handle),
                              file_path.string(),
                              it->second.filepath.string());
                }

                asset_registry_[metadata.handle] = metadata;
                path_to_handle_map_[metadata.filepath] = metadata.handle;
                loaded_count++;

                CORE_TRACE("Registered asset: '{0}' [Handle: {1}]",
                           file_path.string(), static_cast<uint64_t>(metadata.handle));
            }
            else {
                CORE_WARN("Skipped asset '{0}': Failed to parse or generate metadata", file_path.string());
                skipped_count++;
            }
        }

        CORE_INFO("Asset scan complete: {0} assets loaded ({1} skipped), {2} total registered",
                  loaded_count, skipped_count, asset_registry_.size());
    }

    void AssetManager::register_extension(std::string_view extension, AssetType type, std::string_view type_name) {
        MetafileHandler::register_extension(extension, type, type_name);
    }

    void AssetManager::unload(AssetHandle handle) {
        for (auto& asset_pool : pools_ | std::views::values) {
            asset_pool->remove(handle);
        }

        if (auto it = asset_registry_.find(handle); it != asset_registry_.end() && it->second.runtime_only) {
            if (!it->second.filepath.empty()) {
                path_to_handle_map_.erase(it->second.filepath);
            }
            asset_registry_.erase(it);
        }
    }

    void AssetManager::clear_cache() {
        for (auto& asset_pool : pools_ | std::views::values) {
            asset_pool->clear();
        }
    }

    void AssetManager::clear() {
        clear_cache();
        asset_registry_.clear();
        path_to_handle_map_.clear();
        primitives_registered_ = false;
    }

    const AssetMetadata& AssetManager::metadata(AssetHandle handle) const {
        if (auto it = asset_registry_.find(handle); it != asset_registry_.end()) {
            return it->second;
        }
        return null_metadata_;
    }

    const AssetMetadata& AssetManager::metadata(const std::filesystem::path& filepath) const {
        return metadata(handle(filepath));
    }

    AssetHandle AssetManager::handle(const std::filesystem::path& filepath) const {
        if (auto it = path_to_handle_map_.find(normalize_path(filepath)); it != path_to_handle_map_.end()) {
            return it->second;
        }
        return {0};
    }

    std::filesystem::path AssetManager::path(AssetHandle handle) const {
        const auto& meta = metadata(handle);
        return meta.valid() ? meta.filepath : std::filesystem::path{};
    }

    std::filesystem::path AssetManager::source_path(AssetHandle handle) const {
        const auto& meta = metadata(handle);
        return meta.valid() ? meta.shader_source_filepath : std::filesystem::path{};
    }

    std::filesystem::path AssetManager::normalize_path(const std::filesystem::path& filepath) {
        if (filepath.generic_string().find("://") != std::string::npos) {
            return filepath;
        }
        return filepath.lexically_normal();
    }

    void AssetManager::register_primitives() {
        if (primitives_registered_) return;
        primitives_registered_ = true;

        auto register_primitive = [this](const std::string& virtual_path, uint64_t fixed_id) {
            AssetHandle handle(fixed_id);
            std::filesystem::path normalized = normalize_path(virtual_path);
            AssetMetadata meta{
                .handle   = handle,
                .type     = Asset::MESH,
                .filepath = normalized
            };
            asset_registry_[handle] = meta;
            path_to_handle_map_[normalized] = handle;
        };

        register_primitive("primitive://plane", 1);
        register_primitive("primitive://circle", 2);
        register_primitive("primitive://cube", 3);
        register_primitive("primitive://sphere", 4);
        register_primitive("primitive://cylinder", 5);
        register_primitive("primitive://pyramid", 6);
    }
}
