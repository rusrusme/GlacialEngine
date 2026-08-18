//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <scene/Scene.hpp>
#include <scene/ecs/Entity.hpp>

namespace editor {
    class Hierarchy {
    public:
        Hierarchy() = default;

        void init(std::shared_ptr<engine::Scene> scene) { scene_ = std::move(scene); }

        engine::Entity add_entity(const std::string& name);

        void render_ui();

        [[nodiscard]] engine::Entity get_selected_entity() const { return selected_entity_; }

    private:
        std::shared_ptr<engine::Scene> scene_ = nullptr;
        engine::Entity selected_entity_;

        char name_buffer_[128] = "";
    };
}
