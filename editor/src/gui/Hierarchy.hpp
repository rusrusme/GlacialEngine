//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "scene/Scene.hpp"
#include "scene/ecs/Entity.hpp"

namespace editor {
    class Hierarchy {
    public:
        Hierarchy() = default;

        void init(std::shared_ptr<engine::Scene> scene, std::shared_ptr<engine::AssetManager> asset_manager);

        #ifdef ENABLE_IMGUI
        void render_imgui();
        #endif

        engine::Entity add_entity(const std::string& name);

        void set_context(std::shared_ptr<engine::Scene> current_scene);

        [[nodiscard]] engine::Entity get_selected_entity() const { return selected_entity_; }
        void set_selected_entity(const engine::Entity new_entity) { selected_entity_ = new_entity; }

    private:
        std::shared_ptr<engine::Scene> scene_ = nullptr;
        std::shared_ptr<engine::AssetManager> asset_manager_ = nullptr;
        engine::Entity selected_entity_;

        char name_buffer_[128] = "";
    };
}
