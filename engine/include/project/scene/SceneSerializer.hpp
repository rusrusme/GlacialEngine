//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "asset/AssetManager.hpp"
#include "project/scene/Scene.hpp"

namespace engine {
    struct SystemContext;

    class SceneSerializer {
    public:
        SceneSerializer() = default;
        ~SceneSerializer() = default;

        // Disk file
        void serialize(const std::string& filepath, SystemContext& context);
        bool deserialize(const std::string& filepath, SystemContext& context);

        // In-memory Entity serialization API
        static nlohmann::json serialize_entity(Entity entity, SystemContext& context);
        static Entity deserialize_entity(const nlohmann::json& entity_json, SystemContext& context);

        // Math serialization helpers
        static nlohmann::json serialize_vec3(const glm::vec3& v);
        static nlohmann::json serialize_vec4(const glm::vec4& v);

        static glm::vec3 deserialize_vec3(const nlohmann::json& j);

        static glm::vec4 deserialize_vec4(const nlohmann::json& j);
    };
}
