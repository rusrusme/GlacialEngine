//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/vec3.hpp>

namespace engine {
    struct Ray {
        glm::vec3 origin{0.0f};

        glm::vec3 direction{0.0f, 0.0f, -1.0f};

        /** @brief Calculates the point along the ray at a given distance from the origin */
        [[nodiscard]] glm::vec3 at(const float distance) const {
            return origin + direction * distance;
        }

        bool intersect_plane_y(const float plane_y, glm::vec3& intersection_point) const {
            if (std::abs(direction.y) < 1e-6f) return false;

            const float t = (plane_y - origin.y) / direction.y;
            if (t < 0.0f) return false;

            intersection_point = origin + direction * t;
            return true;
        }
    };
}
