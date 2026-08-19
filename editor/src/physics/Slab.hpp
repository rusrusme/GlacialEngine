//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <math/Ray.hpp>

namespace editor {
    struct AABB {
        glm::vec3 min{-0.5f};
        glm::vec3 max{0.5f};

        AABB() = default;
        AABB(const glm::vec3 min, const glm::vec3 max) : min(min), max(max) {}
    };

    inline bool ray_intersects_aabb(const engine::Ray& ray, const AABB& aabb, float& out_t) {
        float t_min = 0.0f;
        float t_max = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i) {
            if (std::abs(ray.direction[i]) < 1e-6f) {
                // Strahl ist parallel zur Achse
                if (ray.origin[i] < aabb.min[i] || ray.origin[i] > aabb.max[i]) {
                    return false;
                }
            }
            else {
                float inv_d = 1.0f / ray.direction[i];
                float t1 = (aabb.min[i] - ray.origin[i]) * inv_d;
                float t2 = (aabb.max[i] - ray.origin[i]) * inv_d;

                if (t1 > t2) std::swap(t1, t2);

                t_min = std::max(t_min, t1);
                t_max = std::min(t_max, t2);

                if (t_min > t_max) return false;
            }
        }

        out_t = t_min;
        return true;
    }
}
