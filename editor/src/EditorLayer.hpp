//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "EditorGrid.hpp"
#include "asset/AssetManager.hpp"
#include "core/Layer.hpp"
#include "input/CameraController.hpp"
#include "render/Renderer.hpp"
#include "scene/Scene.hpp"
#include "ui/UILayer.hpp"
#include "../../engine/src/ui/UISystem.hpp"
#include "core/Framebuffer.hpp"

namespace editor {
    class EditorLayer : public engine::Layer {
    public:
        explicit EditorLayer(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager);
        ~EditorLayer() override = default;

        void on_attach(engine::Renderer& renderer) override;
        void on_detach() override;

        void on_update(float delta_time) override;

        void on_render(engine::Renderer& renderer, float delta_time) override;

        #ifdef ENABLE_IMGUI
        void on_imgui_render(engine::Renderer& renderer, float delta_time) override;
        #endif

        void on_event(engine::Event& event) override;

        [[nodiscard]] const engine::Camera& get_camera() const { return camera_; }

        [[nodiscard]] uint32_t get_scene_texture_id() const {
            return framebuffer_ ? framebuffer_->get_color_attachment_id() : 0;
        }

        [[nodiscard]] uint32_t get_framebuffer_width() const {
            return framebuffer_ ? framebuffer_->get_width() : 0;
        }

        [[nodiscard]] uint32_t get_framebuffer_height() const {
            return framebuffer_ ? framebuffer_->get_height() : 0;
        }

        void resize_viewport(uint32_t width, uint32_t height) {
            if (framebuffer_) {
                framebuffer_->resize(width, height);
                camera_.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
            }
        }

        void set_viewport_hovered(bool hovered) { is_viewport_hovered_ = hovered; }
        void set_viewport_focused(bool focused) { is_viewport_focused_ = focused; }

    private:
        std::unique_ptr<engine::Framebuffer> framebuffer_;
        std::shared_ptr<engine::Scene> active_scene_;
        std::shared_ptr<engine::AssetManager> asset_manager_;

        engine::Camera camera_;
        engine::CameraController camera_controller_;

        EditorGrid editor_grid_;

        bool is_viewport_hovered_ = false;
        bool is_viewport_focused_ = false;

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
        // -------------------------

        static void draw_directional_light(engine::Renderer& renderer, glm::vec3 position, const glm::vec3& direction, const glm::vec3& color);
    };
}
