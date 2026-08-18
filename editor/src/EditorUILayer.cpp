//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorUILayer.hpp"

namespace editor {
    EditorUILayer::EditorUILayer(const std::shared_ptr<engine::Scene>& scene) : active_scene_(scene) {}

    void EditorUILayer::on_attach(engine::Renderer& renderer) {
        object_editor_.init(active_scene_);
        hierarchy_.init(active_scene_);
    }

    void EditorUILayer::on_detach() {
        object_editor_.init(nullptr);
        hierarchy_.init(nullptr);
    }

    void EditorUILayer::on_update(float delta_time) {

    }

    void EditorUILayer::on_render(engine::Renderer& renderer, float delta_time) {
        hierarchy_.render_ui();
        object_editor_.set_selected_entity(hierarchy_.get_selected_entity());
        object_editor_.render_ui();
    }

    void EditorUILayer::on_imGuiRender(engine::Renderer& renderer, float delta_time) {
    }

    void EditorUILayer::on_event(engine::Event& event) {

    }
}
