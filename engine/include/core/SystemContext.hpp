//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

namespace engine {
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
        AudioSystem& audio;
        AssetManager& assets;
        SceneManager& scenes;

        ShaderLibrary& shaders;
        // TODO
        // SceneRenderer& renderer;
        // GPUMeshManager& mesh_manager;
        // MaterialSystem& materials;
        // TextureSystem& textures;

        Window& window;

        std::shared_ptr<Project> current_project{nullptr};

        SystemContext(AudioSystem& audio_system,
                      AssetManager& asset_manager,
                      SceneManager& scene_manager,
                      ShaderLibrary& shader_library,
                      Window& window)
            : audio(audio_system),
              assets(asset_manager),
              scenes(scene_manager),
              shaders(shader_library),
              window(window) {}
    };
}
