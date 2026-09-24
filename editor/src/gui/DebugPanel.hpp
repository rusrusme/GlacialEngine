//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace editor {
    class DebugPanel {
    public:
        DebugPanel() = default;

        #ifdef ENABLE_IMGUI
        void render_imgui(engine::Renderer& renderer, float delta_time);
        #endif

        void toggle_debug_panel() { show_debug_panel_ = !show_debug_panel_; }
        [[nodiscard]] bool is_debug_panel_enabled() const { return show_debug_panel_; }
        void set_debug_panel_enabled(const bool enabled) { show_debug_panel_ = enabled; }

        [[nodiscard]] bool is_grid_enabled() const { return show_grid_; }
        void set_grid_enabled(const bool toggle) { show_grid_ = toggle; }

    private:
        // -----------------------------------------
        // Editor overlays
        // -----------------------------------------
        bool show_grid_ = true;
        bool show_gizmo_ = true;
        bool show_debug_panel_ = true;

        // -------------------------
        // Debug controls
        // -------------------------

        void set_gizmo_enabled(const bool toggle) { show_gizmo_ = toggle; }
        [[nodiscard]] bool is_gizmo_enabled() const { return show_gizmo_; }
        // -------------------------
    };
}
