//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"
#include "event/Event.hpp"
#include "render/Camera.hpp"
#include "render/Renderer.hpp"
#include "ui/AssetBrowser.hpp"
#include "ui/Hierarchy.hpp"
#include "ui/ObjectInspector.hpp"

#ifdef ENABLE_IMGUI
struct ImVec2;
#endif

namespace editor {
    class EditorLayer;

    class EditorUILayer : public engine::Layer {
    public:
        explicit EditorUILayer(std::shared_ptr<engine::Scene>& scene, std::shared_ptr<engine::AssetManager> asset_manager);
        ~EditorUILayer() override = default;

        void on_attach(engine::Renderer& renderer) override;
        void on_detach() override;
        void on_update(float delta_time) override;
        void on_render(engine::Renderer& renderer, float delta_time) override;

        #ifdef ENABLE_IMGUI
        void on_imgui_render(engine::Renderer& renderer, float delta_time) override;
        #endif

        void on_event(engine::Event& event) override;

        void set_camera(const engine::Camera* camera) { camera_ = camera; }
        void set_editor(EditorLayer* editor_layer) { editor_layer_ = editor_layer; }

    private:
        #ifdef ENABLE_IMGUI
        void draw_imguizmo(engine::Entity selected_entity, const engine::Camera& camera, const ImVec2& viewport_pos, const ImVec2& viewport_size);
        void draw_imgui_menu_bar() const;

        bool is_viewport_hovered_ = false;
        bool is_viewport_focused_ = false;
        ImVec2 viewport_bounds_min_{0.0f, 0.0f};
        glm::vec2 viewport_bounds_size_{0.0f, 0.0f};
        #endif

        engine::TransformComponent initial_transform_{};
        bool was_using_gizmo_ = false;
        bool gizmo_canceled_ = false;
        glm::mat4 active_gizmo_matrix_{1.0f};

        std::shared_ptr<engine::Scene> active_scene_ = nullptr;
        std::shared_ptr<engine::AssetManager> asset_manager_ = nullptr;

        const engine::Camera* camera_ = nullptr;
        EditorLayer* editor_layer_ = nullptr;

        ObjectInspector object_inspector_;
        Hierarchy hierarchy_;
        AssetBrowser asset_browser_;
    };
}
