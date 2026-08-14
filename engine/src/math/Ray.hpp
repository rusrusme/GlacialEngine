//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/vec3.hpp>

namespace engine {
    struct Ray {
        /** @brief Starting point of ray in 3d space */
        glm::vec3 origin;

        /** @brief Normalized direction vector of ray */
        glm::vec3 direction;

        /** @brief Calculates the point along the ray at a given distance from the origin */
        [[nodiscard]] glm::vec3 at(const float distance) const {
            return origin + direction * distance;
        }
    };
}
