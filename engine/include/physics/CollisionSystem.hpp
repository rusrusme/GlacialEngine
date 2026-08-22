//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <optional>

#include "physics/SpatialHashGrid.hpp"
#include "scene/ecs/Entity.hpp"

namespace engine {
    struct AABB;
    struct Ray;
    class Scene;
    class AssetManager;

    struct RaycastHit {
        Entity entity{};
        glm::vec3 point{0.0f};
        float distance = 0.0f;
    };

    class CollisionSystem {
    public:
        explicit CollisionSystem(float cell_size = 10.0f);

        void update_broadphase(Scene& scene);

        [[nodiscard]] static std::optional<RaycastHit> raycast(Scene& scene, const Ray& ray, float max_distance = 1000.0f, AssetManager* asset_manager = nullptr);

        [[nodiscard]] bool intersects_any(Scene& scene, const AABB& target_aabb) const;

        [[nodiscard]] std::vector<Entity> query_aabb(Scene& scene, const AABB& target_aabb) const;

    private:
        SpatialHashGrid spatial_grid_;
    };
}
