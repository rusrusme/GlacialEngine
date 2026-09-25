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
        void render_imgui(const std::shared_ptr<engine::Scene>& active_scene, const std::shared_ptr<engine::AssetManager>& asset_manager);
        #endif
    };
}
