//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "audio/AudioLayer.hpp"
#include "scene/Scene.hpp"
#include "scene/ecs/Entity.hpp"

namespace editor {
    class ObjectInspector {
    public:
        ObjectInspector() = default;

        void init(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager, engine::AudioLayer* audio_layer);

        #ifdef ENABLE_IMGUI
        void render_imgui();
        #endif

        void set_context(std::shared_ptr<engine::Scene> current_scene) { scene_ = std::move(current_scene); }

        void set_selected_entity(const engine::Entity entity) { selected_entity_ = entity; }

    private:
        #ifdef ENABLE_IMGUI
        void draw_component(engine::Entity entity) const;
        #endif

        std::shared_ptr<engine::Scene> scene_ = nullptr;
        std::shared_ptr<engine::AssetManager> asset_manager_ = nullptr;
        engine::AudioLayer* audio_layer_ = nullptr;

        engine::Entity selected_entity_{};
    };
}
