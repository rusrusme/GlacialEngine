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
        push_layer(std::make_unique<EditorLayer>(active_scene_));
        push_layer(std::make_unique<EditorUILayer>(active_scene_));
    }
}

int main() {
    editor::Editor engine;
    return engine.run();
}
