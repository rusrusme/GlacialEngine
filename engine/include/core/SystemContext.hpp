//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    class AudioSystem;
    class AssetManager;
    class Renderer;
    class SceneManager;

    struct SystemContext {
        Renderer& renderer;
        AudioSystem& audio;
        AssetManager& assets;
        SceneManager& scenes;
    };
}