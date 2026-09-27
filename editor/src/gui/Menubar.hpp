//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

namespace engine {
    class Scene;
    class AssetManager;
}

namespace editor {
    class Menubar {
    public:
        Menubar();
        ~Menubar();

        #ifdef ENABLE_IMGUI
        void render_imgui(std::shared_ptr<engine::Scene> scene, engine::AssetManager& assets);
        #endif
    };
}
