//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include <EditorGrid.hpp>
#include <memory>
#include <core/Layer.hpp>
#include <input/CameraController.hpp>
#include <render/Renderer.hpp>
#include <scene/Scene.hpp>

namespace editor {
    class EditorLayer : public engine::Layer {
    public:
        EditorLayer() = default;
        ~EditorLayer() override = default;

        void on_attach(engine::Renderer& renderer) override;
        void on_detach() override;

        void on_update(float delta_time) override;

        void on_render(engine::Renderer& renderer, float delta_time) override;
        void on_imGuiRender(engine::Renderer& renderer, float delta_time) override;

        void on_event(engine::Event& event) override;

    private:
        std::shared_ptr<engine::Scene> active_scene_;
        engine::Camera camera_;
        engine::CameraController camera_controller_;

        EditorGrid editor_grid_;

        // -----------------------------------------
        glm::vec3 hover_tile_pos_{0.0f};
        bool is_hovering_tile_ = false;
        // -----------------------------------------

        // -----------------------------------------
        // Editor overlays
        // -----------------------------------------
        bool show_grid_ = true;
        bool show_gizmo_ = true;
        bool show_debug_panel_ = true;

        // -------------------------
        // Debug controls
        // -------------------------
        void set_grid_enabled(const bool toggle) { show_grid_ = toggle; }
        [[nodiscard]] bool is_grid_enabled() const { return show_grid_; }

        void set_gizmo_enabled(const bool toggle) { show_gizmo_ = toggle; }
        [[nodiscard]] bool is_gizmo_enabled() const { return show_gizmo_; }

        [[nodiscard]] bool is_debug_panel_enabled() const { return show_debug_panel_; }
        void set_debug_panel_enabled(const bool enabled) { show_debug_panel_ = enabled; }
        void toggle_debug_panel() { show_debug_panel_ = !show_debug_panel_; }

        // -------------------------
        float total_time_ = 0.0f;

        void cube_grid(engine::Renderer& renderer) const;
        void orbiting_spheres(engine::Renderer& renderer) const;
        void dynamic_ring(engine::Renderer& renderer) const;
        void wave_around_scene(engine::Renderer& renderer) const;
        void tree_trunks_around_pyramid(engine::Renderer& renderer) const;
    };
}
