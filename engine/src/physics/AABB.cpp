//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "physics/AABB.hpp"

#include "math/Ray.hpp"
#include "render/Frustum.hpp"

namespace engine {
    bool AABB::intersects_aabb(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z);
    }

    bool AABB::intersects_ray(const Ray& ray, float& out_t) const {
        float t_min = 0.0f;
        float t_max = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i) {
            if (std::abs(ray.direction[i]) < 1e-6f) {
                if (ray.origin[i] < min[i] || ray.origin[i] > max[i]) {
                    return false;
                }
            }
            else {
                float inv_d = 1.0f / ray.direction[i];
                float t1 = (min[i] - ray.origin[i]) * inv_d;
                float t2 = (max[i] - ray.origin[i]) * inv_d;

                if (t1 > t2) std::swap(t1, t2);

                t_min = std::max(t_min, t1);
                t_max = std::min(t_max, t2);

                if (t_min > t_max) return false;
            }
        }
        out_t = t_min;
        return true;
    }

    bool AABB::intersects_sphere(const glm::vec3& center, float radius) const {
        const glm::vec3 closest = glm::clamp(center, min, max);
        const glm::vec3 diff = center - closest;
        return glm::dot(diff, diff) <= (radius * radius);
    }

    bool AABB::intersects_sphere(const glm::vec3& center, float radius, const glm::mat4& transform) const {
        const glm::vec3 local_center = (min + max) * 0.5f;
        const glm::vec3 extents = (max - min) * 0.5f;

        const auto world_center = glm::vec3(transform * glm::vec4(local_center, 1.0f));

        const auto abs_matrix = glm::mat3(
            glm::abs(glm::vec3(transform[0])),
            glm::abs(glm::vec3(transform[1])),
            glm::abs(glm::vec3(transform[2]))
        );
        const glm::vec3 world_extents = abs_matrix * extents;

        const glm::vec3 closest = glm::clamp(center, world_center - world_extents, world_center + world_extents);
        const glm::vec3 diff = center - closest;

        return glm::dot(diff, diff) <= (radius * radius);
    }
}
