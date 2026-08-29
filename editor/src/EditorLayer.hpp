//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "asset/AssetManager.hpp"
#include "core/Framebuffer.hpp"
#include "core/Layer.hpp"
#include "environment/EditorGrid.hpp"
#include "input/CameraController.hpp"
#include "panel/DebugPanel.hpp"
#include "render/Renderer.hpp"
#include "scene/Scene.hpp"

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

        [[nodiscard]] uint32_t get_scene_texture_id() const { return framebuffer_ ? framebuffer_->get_color_attachment_id() : 0; }
        [[nodiscard]] uint32_t get_framebuffer_width() const { return framebuffer_ ? framebuffer_->get_width() : 0; }
        [[nodiscard]] uint32_t get_framebuffer_height() const { return framebuffer_ ? framebuffer_->get_height() : 0; }

        void resize_viewport(uint32_t width, uint32_t height);

        void set_viewport_hovered(const bool hovered) { is_viewport_hovered_ = hovered; }
        void set_viewport_focused(const bool focused) { is_viewport_focused_ = focused; }

    private:
        std::unique_ptr<engine::Framebuffer> framebuffer_;
        std::shared_ptr<engine::Scene> active_scene_;
        std::shared_ptr<engine::AssetManager> asset_manager_;
        std::unique_ptr<DebugPanel> debug_panel_;

        engine::Camera camera_;
        engine::CameraController camera_controller_;

        // -------------------------
        float total_time_ = 0.0f;

        EditorGrid editor_grid_;

        bool is_viewport_hovered_ = false;
        bool is_viewport_focused_ = false;

        static void draw_directional_light(engine::Renderer& renderer, glm::vec3 position, const glm::vec3& direction, const glm::vec3& color);
    };
}
