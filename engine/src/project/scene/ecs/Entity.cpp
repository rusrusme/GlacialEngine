//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/ecs/Entity.hpp"

namespace engine {
    Entity::Entity(const entt::entity handle, Scene* scene)
        : entity_handle_(handle), scene_(scene) {
    }
}
