//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <core/Layer.hpp>
#include <event/Event.hpp>
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

    private:
        std::shared_ptr<engine::Scene> active_scene_;

        ObjectInspector object_editor_;
        Hierarchy hierarchy_;
    };
}
