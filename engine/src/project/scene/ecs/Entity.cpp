//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/ecs/Entity.hpp"

namespace engine {
    Entity::Entity(const entt::entity handle, Scene* scene)
        : entity_handle_(handle), scene_(scene) {
    }

    bool Entity::is_active() const {
        return !scene_->registry_.all_of<DisabledComponent>(entity_handle_);
    }

    void Entity::set_active(bool active) const {
        if (active) {
            scene_->registry_.remove<DisabledComponent>(entity_handle_);
        }
        else {
            scene_->registry_.emplace_or_replace<DisabledComponent>(entity_handle_);
        }
    }

    UUID Entity::uuid() { return get_component<IDComponent>().id; }

    const std::string& Entity::name() { return get_component<LabelComponent>().tag; }
}
