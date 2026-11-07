//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <nlohmann/json.hpp>

#include "project/scene/Scene.hpp"
#include "project/scene/SceneSerializer.hpp"
#include "project/scene/ecs/Entity.hpp"

namespace editor {
    using namespace engine;
    using json = nlohmann::json;

    struct EntitySnapshot {
        // Serializes all components of a single entity
        static json serialize(Entity entity, SystemContext& context) {
            return SceneSerializer::serialize_entity(entity, context);
        }

        // Restores an entity from JSON
        static Entity deserialize(const json& entity, SystemContext& context) {
            return SceneSerializer::deserialize_entity(entity, context);
        }
    };
}