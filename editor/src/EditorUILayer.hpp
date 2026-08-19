//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <imGUi.h>
#include <core/Layer.hpp>
#include <event/Event.hpp>
#include <render/Camera.hpp>
#include <render/Renderer.hpp>
#include <ui/Hierarchy.hpp>
#include <ui/ObjectInspector.hpp>

namespace editor {
    class EditorUILayer : public engine::Layer {
    public:
        explicit EditorUILayer(const std::shared_ptr<engine::Scene>& scene);
        ~EditorUILayer() override = default;

        void on_attach(engine::Renderer& renderer) override;
        void on_detach() override;
        void on_update(float delta_time) override;
        void on_render(engine::Renderer& renderer, float delta_time) override;
        void on_imGuiRender(engine::Renderer& renderer, float delta_time) override;

        void on_event(engine::Event& event) override;

        void set_camera(const engine::Camera* camera) { camera_ = camera; }

    private:
        void draw_gizmo(engine::Entity selected_entity, const engine::Camera& camera, const ImVec2& viewport_pos, const ImVec2& viewport_size);

        bool block_gizmo_until_mouse_up_ = false;
        engine::TransformComponent initial_transform_{};
        bool was_using_gizmo_ = false;
        bool gizmo_canceled_ = false;

        std::shared_ptr<engine::Scene> active_scene_;
        const engine::Camera* camera_ = nullptr;

        ObjectInspector object_editor_;
        Hierarchy hierarchy_;
    };
}
