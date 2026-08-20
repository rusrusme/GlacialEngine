//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <scene/Scene.hpp>
#include <scene/ecs/Entity.hpp>

namespace editor {
    class ObjectInspector {
    public:
        ObjectInspector() = default;

        void init(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager);
        void set_selected_entity(engine::Entity entity) { selected_entity_ = entity; }

        void render_ui();

    private:
        void draw_component(engine::Entity entity);

        std::shared_ptr<engine::AssetManager> asset_manager_ = nullptr;
        std::shared_ptr<engine::Scene> scene_ = nullptr;
        engine::Entity selected_entity_{};
    };
}
