//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Editor.hpp"

#include "EditorLayer.hpp"
#include "gui/UILayer.hpp"
#include "audio/AudioLayer.hpp"

namespace editor {
    Editor::Editor() {
        active_scene_ = std::make_shared<engine::Scene>();
        asset_manager_ = std::make_unique<engine::AssetManager>();

        auto audio_layer = std::make_unique<engine::AudioLayer>(active_scene_, context());
        auto editor_layer = std::make_unique<EditorLayer>(active_scene_, asset_manager_);
        auto imgui_layer = std::make_unique<UILayer>(active_scene_, asset_manager_);

        editor_layer->set_audio(audio_layer.get());

        imgui_layer->set_camera(&editor_layer->get_camera());
        imgui_layer->set_editor(editor_layer.get());
        imgui_layer->set_audio(audio_layer.get());

        push_layer(std::move(audio_layer));
        push_layer(std::move(editor_layer));
        push_layer(std::move(imgui_layer));
    }
}

int main() {
    editor::Editor engine;
    return engine.run();
}
