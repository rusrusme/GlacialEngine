//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>
#include <typeindex>
#include <unordered_map>

#include "asset/Asset.hpp"
#include "core/Log.hpp"
#include "render/rhi/RHIDevice.hpp"

namespace engine {
    class AudioSystem;
    class ShaderLibrary;

    class AssetManager {
    public:
        template<typename T>
        using Loader = std::function<std::shared_ptr<T>(AssetManager&, const AssetMetadata&)>;

        void init(RHIDevice& device, ShaderLibrary& shaders, AudioSystem& audio);

        void scan_directory(const std::filesystem::path& asset_directory, bool generate_meta = true);

        void register_extension(std::string_view extension, AssetType type, std::string_view type_name);

        template<typename T>
        void register_loader(Loader<T> loader) {
            loaders_[std::type_index(typeid(T))] =
                [fn = std::move(loader)](AssetManager& assets, const AssetMetadata& meta) -> std::shared_ptr<void> {
                    return fn(assets, meta);
                };
        }

        template<typename T>
        AssetHandle emplace(std::shared_ptr<T> asset,
                            AssetType type = AssetType(typeid(T).name()),
                            const std::filesystem::path& virtual_path = {}) {
            if (!asset) return {0};

            AssetHandle handle;
            AssetMetadata meta{
                .handle       = handle,
                .type         = type.valid() ? type : AssetType(typeid(T).name()),
                .filepath     = virtual_path.empty() ? std::filesystem::path{} : normalize_path(virtual_path),
                .runtime_only = true
            };

            asset_registry_[handle] = meta;
            if (!meta.filepath.empty()) {
                path_to_handle_map_[meta.filepath] = handle;
            }

            pool<T>().items[handle] = std::move(asset);
            return handle;
        }

        template<typename T>
        std::shared_ptr<T> get(AssetHandle handle) {
            if (handle == 0) return nullptr;

            auto& typed_pool = pool<T>();
            if (auto it = typed_pool.items.find(handle); it != typed_pool.items.end()) {
                return it->second;
            }

            const auto& meta = metadata(handle);
            if (!meta.valid() || meta.runtime_only) {
                return nullptr;
            }

            auto loader_it = loaders_.find(std::type_index(typeid(T)));
            if (loader_it == loaders_.end()) {
                CORE_ERROR("[AssetManager] No loader registered for asset '{0}'", meta.filepath.string());
                return nullptr;
            }

            auto loaded = std::static_pointer_cast<T>(loader_it->second(*this, meta));
            if (loaded) {
                typed_pool.items[handle] = loaded;
            }
            return loaded;
        }

        template<typename T>
        std::shared_ptr<T> get(const std::filesystem::path& filepath) {
            return get<T>(handle(filepath));
        }

        template<typename T>
        [[nodiscard]] bool has(AssetHandle handle) const {
            const auto* typed_pool = find_pool<T>();
            return typed_pool && typed_pool->items.contains(handle);
        }

        void unload(AssetHandle handle);
        void clear_cache();
        void clear();

        [[nodiscard]] RHIDevice* device() const { return device_; }
        [[nodiscard]] ShaderLibrary* shaders() const { return shaders_; }
        [[nodiscard]] AudioSystem* audio() const { return audio_; }
        [[nodiscard]] bool contains(AssetHandle handle) const { return asset_registry_.contains(handle); }
        [[nodiscard]] const AssetMetadata& metadata(AssetHandle handle) const;
        [[nodiscard]] const AssetMetadata& metadata(const std::filesystem::path& filepath) const;
        [[nodiscard]] AssetHandle handle(const std::filesystem::path& filepath) const;
        [[nodiscard]] std::filesystem::path path(AssetHandle handle) const;
        [[nodiscard]] std::filesystem::path compiled_path(AssetHandle handle) const;
        [[nodiscard]] const std::unordered_map<AssetHandle, AssetMetadata>& registry() const { return asset_registry_; }

    private:
        struct AssetPool {
            virtual ~AssetPool() = default;
            virtual void remove(AssetHandle handle) = 0;
            virtual void clear() = 0;
        };

        template<typename T>
        struct TypedAssetPool : AssetPool {
            std::unordered_map<AssetHandle, std::shared_ptr<T>> items;

            void remove(AssetHandle handle) override { items.erase(handle); }
            void clear() override { items.clear(); }
        };

        template<typename T>
        TypedAssetPool<T>& pool() {
            const std::type_index key(typeid(T));
            auto it = pools_.find(key);
            if (it == pools_.end()) {
                auto created = std::make_unique<TypedAssetPool<T>>();
                auto* ptr = created.get();
                pools_[key] = std::move(created);
                return *ptr;
            }
            return static_cast<TypedAssetPool<T>&>(*it->second);
        }

        template<typename T>
        const TypedAssetPool<T>* find_pool() const {
            auto it = pools_.find(std::type_index(typeid(T)));
            if (it == pools_.end()) return nullptr;
            return static_cast<const TypedAssetPool<T>*>(it->second.get());
        }

        static std::filesystem::path normalize_path(const std::filesystem::path& filepath);
        void register_primitives();

        using ErasedLoader = std::function<std::shared_ptr<void>(AssetManager&, const AssetMetadata&)>;

        RHIDevice* device_{nullptr};
        ShaderLibrary* shaders_{nullptr};
        AudioSystem* audio_{nullptr};
        bool primitives_registered_{false};

        std::unordered_map<AssetHandle, AssetMetadata> asset_registry_;
        std::unordered_map<std::filesystem::path, AssetHandle> path_to_handle_map_;
        std::unordered_map<std::type_index, std::unique_ptr<AssetPool>> pools_;
        std::unordered_map<std::type_index, ErasedLoader> loaders_;

        inline static AssetMetadata null_metadata_{};
    };
}
