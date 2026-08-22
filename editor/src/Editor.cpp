//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Editor.hpp"

#include "EditorLayer.hpp"
#include "EditorUILayer.hpp"

namespace editor {
    Editor::Editor() {
        active_scene_ = std::make_shared<engine::Scene>();
        asset_manager_ = std::make_unique<engine::AssetManager>();

        auto editor_layer = std::make_unique<EditorLayer>(active_scene_, asset_manager_);
        auto imgui_layer = std::make_unique<EditorUILayer>(active_scene_, asset_manager_);

        imgui_layer->set_camera(&editor_layer->get_camera());
        imgui_layer->set_editor(editor_layer.get());

        push_layer(std::move(editor_layer));
        push_layer(std::move(imgui_layer));
    }
}

int main() {
    editor::Editor engine;
    return engine.run();
}
