//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace engine {
    class RenderContext;
    class Window;
    class AudioSystem;
    class AssetManager;
    class Project;
    class SceneManager;
    class SceneRenderer;
    class ShaderLibrary;
    class GPUMeshManager;
    class MaterialSystem;
    class TextureSystem;

    struct SystemContext {
        /// Engine core systems
        AudioSystem& audio;
        AssetManager& assets;
        SceneManager& scenes;

        ShaderLibrary& shaders;
        RenderContext& render_context; // TODO Abstraction leak -> Move to SceneRenderer
        SceneRenderer& renderer;
        // TODO
        // GPUMeshManager& mesh_manager;
        // MaterialSystem& materials;
        // TextureSystem& textures;

        Window& window;

        std::shared_ptr<Project> current_project{nullptr};

        SystemContext(AudioSystem& audio_system,
                      AssetManager& asset_manager,
                      SceneManager& scene_manager,
                      ShaderLibrary& shader_library,
                      RenderContext& render_context,
                      SceneRenderer& scene_renderer,
                      Window& window)
            : audio(audio_system),
              assets(asset_manager),
              scenes(scene_manager),
              shaders(shader_library),
              render_context(render_context),
              renderer(scene_renderer),
              window(window) {}

        /// Dynamic user-application services
        template<typename T, typename... Args>
        T& emplace(Args&&... args) {
            const auto type = std::type_index(typeid(T));
            auto instance = std::make_shared<T>(std::forward<Args>(args)...);
            user_systems_[type] = instance;
            return *instance;
        }

        template<typename T>
        [[nodiscard]] T& get() {
            const auto it = user_systems_.find(std::type_index(typeid(T)));
            assert(it != user_systems_.end() && "Requested service was not registered");
            return *static_cast<T*>(it->second.get());
        }

        template<typename T>
        [[nodiscard]] bool has() const noexcept {
            return user_systems_.contains(std::type_index(typeid(T)));
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<void>> user_systems_;
    };
}
