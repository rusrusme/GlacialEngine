//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

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
        AudioSystem& audio;
        AssetManager& assets;
        SceneManager& scenes;

        ShaderLibrary& shaders;
        RenderContext& render_context;
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
    };
}
