//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Editor.hpp"

#include <EditorLayer.hpp>
#include <EditorUILayer.hpp>

namespace editor {
    Editor::Editor() {
        active_scene_ = std::make_shared<engine::Scene>();
        auto editor_layer = std::make_unique<EditorLayer>(active_scene_);
        auto ui_layer = std::make_unique<EditorUILayer>(active_scene_);

        ui_layer->set_camera(&editor_layer->get_camera());

        push_layer(std::move(editor_layer));
        push_layer(std::move(ui_layer));
    }
}

int main() {
    editor::Editor engine;
    return engine.run();
}
