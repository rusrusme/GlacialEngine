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
        static void file_menu(const engine::SystemContext& context);
        static void edit_menu(const engine::SystemContext& context);
        static void window_menu(const engine::SystemContext& context);
        static void help_menu(const engine::SystemContext& context);
        void render_imgui(const engine::SystemContext& context);
        #endif
    };
}
