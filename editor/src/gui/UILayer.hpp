//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "Console.hpp"
#include "Menubar.hpp"
#include "core/Layer.hpp"
#include "event/Event.hpp"
#include "gui/AssetBrowser.hpp"
#include "gui/Hierarchy.hpp"
#include "gui/ObjectInspector.hpp"
#include "gui/Toolbar.hpp"
#include "render/Camera.hpp"
#include "render/Renderer.hpp"

#ifdef ENABLE_IMGUI
namespace engine {
    class AudioLayer;
}

struct ImVec2;
#endif

namespace editor {
    struct ToolbarSlot;
    class EditorLayer;

    class UILayer : public engine::Layer {
    public:
        explicit UILayer(std::shared_ptr<engine::Scene>& scene, std::shared_ptr<engine::AssetManager> asset_manager);
        ~UILayer() override = default;

        void on_attach(engine::SystemContext& context) override;
        void on_detach(engine::SystemContext& context) override;
        void on_update(engine::SystemContext& context, float delta_time) override;

        #ifdef ENABLE_IMGUI
        void on_imgui_render(engine::SystemContext& context, float delta_time) override;
        #endif

        void set_camera(const engine::Camera* camera) { camera_ = camera; }
        void set_editor(EditorLayer* editor_layer) { editor_layer_ = editor_layer; }
        void set_audio(engine::AudioLayer* audio_layer) { audio_layer_ = audio_layer; }
        void set_scene_context(const std::shared_ptr<engine::Scene>& current_scene);

    private:
        #ifdef ENABLE_IMGUI
        void draw_imguizmo(engine::Entity selected_entity, const engine::Camera& camera, const ImVec2& viewport_pos, const ImVec2& viewport_size);

        Menubar menubar_{};
        Toolbar toolbar_{};

        bool is_viewport_hovered_ = false;
        bool is_viewport_focused_ = false;
        ImVec2 viewport_bounds_min_{0.0f, 0.0f};
        glm::vec2 viewport_bounds_size_{0.0f, 0.0f};
        #endif

        // Gizmo
        engine::TransformComponent initial_transform_{};
        bool was_using_gizmo_ = false;
        bool gizmo_canceled_ = false;
        glm::mat4 active_gizmo_matrix_{1.0f};

        std::shared_ptr<engine::Scene> active_scene_ = nullptr;
        std::shared_ptr<engine::AssetManager> asset_manager_ = nullptr;

        const engine::Camera* camera_ = nullptr;
        EditorLayer* editor_layer_ = nullptr;
        engine::AudioLayer* audio_layer_ = nullptr;

        ObjectInspector object_inspector_;
        Hierarchy hierarchy_;
        AssetBrowser asset_browser_;
        Console console_;
    };
}
