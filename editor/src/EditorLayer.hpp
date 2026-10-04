//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "command/CommandHistory.hpp"
#include "core/Layer.hpp"
#include "environment/Grid.hpp"
#include "input/CameraController.hpp"
#include "panel/DebugPanel.hpp"
#include "project/scene/Scene.hpp"

namespace engine {
    class AudioSystem;
}

namespace editor {
    class EditorLayer : public engine::Layer {
    public:
        explicit EditorLayer(std::shared_ptr<engine::Scene> scene);
        ~EditorLayer() override = default;

        void on_attach(engine::SystemContext& context) override;
        void on_detach(engine::SystemContext& context) override;

        void on_update(engine::SystemContext& context, float delta_time) override;

        void on_render(engine::SystemContext& context, float delta_time) override;

        #ifdef ENABLE_IMGUI
        void on_imgui_render(engine::SystemContext& context, float delta_time) override;
        #endif

        void on_event(engine::SystemContext& context, engine::Event& event) override;

        [[nodiscard]] engine::Camera& get_camera() { return camera_; }

        void set_viewport_hovered(const bool hovered) { is_viewport_hovered_ = hovered; }
        void set_viewport_focused(const bool focused) { is_viewport_focused_ = focused; }

        [[nodiscard]] CommandHistory& get_command_history() { return command_history_; }
        [[nodiscard]] const CommandHistory& get_command_history() const { return command_history_; }

    private:
        std::shared_ptr<engine::Scene> initial_scene_ = nullptr;

        engine::Camera camera_;
        engine::CameraController camera_controller_;

        float total_time_ = 0.0f;

        DebugPanel debug_panel_{};
        Grid editor_grid_{};
        CommandHistory command_history_{100};

        bool is_viewport_hovered_ = false;
        bool is_viewport_focused_ = false;

        static void draw_directional_light(glm::vec3 position, const glm::vec3& direction, const glm::vec3& color);
    };
}
