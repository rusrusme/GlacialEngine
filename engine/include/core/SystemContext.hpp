//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

namespace engine {
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

        std::shared_ptr<Project> current_project;
    };
}
