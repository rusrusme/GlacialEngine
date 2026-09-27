//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Editor.hpp"

#include "EditorLayer.hpp"
#include "gui/UILayer.hpp"

namespace editor {
    Editor::Editor() {
        auto scene = std::make_shared<engine::Scene>();
        if (!scene) {
            CLIENT_ERROR("Could not create editor scene");
            return;
        }
        auto editor_layer = std::make_unique<EditorLayer>(scene);
        auto imgui_layer = std::make_unique<UILayer>();

        imgui_layer->set_camera(editor_layer->get_camera());
        imgui_layer->set_editor(*editor_layer);

        push_layer(std::move(editor_layer));
        push_layer(std::move(imgui_layer));
    }
}

int main() {
    editor::Editor engine;
    return engine.run();
}
