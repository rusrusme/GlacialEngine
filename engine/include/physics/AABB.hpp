//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/vec3.hpp>

#include "render/Frustum.hpp"

namespace engine {
    struct Ray;

    struct AABB {
        glm::vec3 min{-0.5f};
        glm::vec3 max{0.5f};

        AABB() = default;
        AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

        [[nodiscard]] bool intersects_aabb(const AABB& other) const;

        [[nodiscard]] bool intersects_ray(const Ray& ray, float& out_t) const;

        [[nodiscard]] bool intersects_sphere(const glm::vec3& center, float radius) const;

        [[nodiscard]] bool intersects_sphere(const glm::vec3& center, float radius, const glm::mat4& transform) const;
    };
}
