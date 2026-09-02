//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <array>
#include <cmath>
#include <glm/glm.hpp>

namespace engine {
    struct AABB;

    struct Plane {
        glm::vec3 normal{0.0f, 1.0f, 0.0f};
        float distance{0.0f};

        void normalize() {
            const float length = glm::length(normal);
            if (length > 0.0f) {
                normal /= length;
                distance /= length;
            }
        }
    };

    struct Frustum {
        enum PlaneIndex {
            Left = 0,
            Right,
            Bottom,
            Top,
            Near,
            Far,
            Count
        };

        std::array<Plane, PlaneIndex::Count> planes;

        static Frustum from_view_projection(const glm::mat4& vp);

        [[nodiscard]] bool intersects(const AABB& world_aabb) const;

        // Test against a Local Mesh AABB transformed by a Model Matrix
        [[nodiscard]] bool intersects(const AABB& local_bounds, const glm::mat4& transform) const;
    };
}
