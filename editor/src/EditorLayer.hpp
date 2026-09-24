//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "asset/AssetManager.hpp"
#include "core/Layer.hpp"
#include "environment/Grid.hpp"
#include "input/CameraController.hpp"
#include "gui/DebugPanel.hpp"
#include "render/Renderer.hpp"
#include "scene/Scene.hpp"

namespace engine {
    class AudioLayer;
}

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

        void set_audio(engine::AudioLayer* audio_layer) { audio_layer_ = audio_layer; }

        [[nodiscard]] const engine::Camera& get_camera() const { return camera_; }

        void set_viewport_hovered(const bool hovered) { is_viewport_hovered_ = hovered; }
        void set_viewport_focused(const bool focused) { is_viewport_focused_ = focused; }

    private:
        std::shared_ptr<engine::Scene> active_scene_;
        std::shared_ptr<engine::AssetManager> asset_manager_;
        std::unique_ptr<DebugPanel> debug_panel_;

        engine::AudioLayer* audio_layer_ = nullptr;

        engine::Camera camera_;
        engine::CameraController camera_controller_;

        // -------------------------
        float total_time_ = 0.0f;

        Grid editor_grid_;

        bool is_viewport_hovered_ = false;
        bool is_viewport_focused_ = false;

        static void draw_directional_light(glm::vec3 position, const glm::vec3& direction, const glm::vec3& color);
    };
}
