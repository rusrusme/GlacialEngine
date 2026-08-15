#pragma once


#include <core/Engine.hpp>

#include "EditorLayer.hpp"

namespace editor {
    class Editor : public engine::Application {
    public:
        Editor() = default;
        ~Editor() override = default;

        void on_init(engine::Renderer& renderer) override;

        void on_update(engine::Renderer& renderer, GLFWwindow* window, engine::CameraController& camera_controller, const float delta_time) override;

        void on_render(engine::Renderer& renderer) override;

    private:
        EditorLayer editor_layer_;

        float total_time_ = 0.0f;

        void cube_grid(engine::Renderer& renderer) const;

        void orbiting_spheres(engine::Renderer& renderer) const;

        void dynamic_ring(engine::Renderer& renderer) const;
    };
}