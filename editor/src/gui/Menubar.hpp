//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

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
        void file_menu(const engine::SystemContext& context);
        void edit_menu(const engine::SystemContext& context);
        void window_menu(const engine::SystemContext& context);
        void help_menu(const engine::SystemContext& context);
        void render_imgui(const engine::SystemContext& context);
        #endif
    };
}
