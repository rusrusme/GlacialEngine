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
    // class Renderer;
    class SceneManager;

    struct SystemContext {
        // Renderer& renderer;
        // Todo render: renderer missing -> replace with new architecture
        AudioSystem& audio;
        AssetManager& assets;
        SceneManager& scenes;
        std::shared_ptr<Project> current_project;
    };
}
