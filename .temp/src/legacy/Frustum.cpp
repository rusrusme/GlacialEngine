//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/Frustum.hpp"

#include "physics/AABB.hpp"

namespace engine::legacy {
    using namespace engine;
    Frustum Frustum::from_view_projection(const glm::mat4& vp) {
        Frustum frustum;

        // Left: Row 3 + Row 0
        frustum.planes[Left].normal = glm::vec3(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0]);
        frustum.planes[Left].distance = vp[3][3] + vp[3][0];
        // Right: Row 3 - Row 0
        frustum.planes[Right].normal = glm::vec3(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0]);
        frustum.planes[Right].distance = vp[3][3] - vp[3][0];
        // Bottom: Row 3 + Row 1
        frustum.planes[Bottom].normal = glm::vec3(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1]);
        frustum.planes[Bottom].distance = vp[3][3] + vp[3][1];
        // Top: Row 3 - Row 1
        frustum.planes[Top].normal = glm::vec3(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1]);
        frustum.planes[Top].distance = vp[3][3] - vp[3][1];
        // Near: Row 3 + Row 2 (OpenGL clip depth: [-1, 1])
        frustum.planes[Near].normal = glm::vec3(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2]);
        frustum.planes[Near].distance = vp[3][3] + vp[3][2];
        // Far: Row 3 - Row 2
        frustum.planes[Far].normal = glm::vec3(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2]);
        frustum.planes[Far].distance = vp[3][3] - vp[3][2];

        for (auto& plane : frustum.planes) {
            plane.normalize();
        }
        return frustum;
    }

    bool Frustum::intersects(const AABB& world_aabb) const {
        const glm::vec3 center = (world_aabb.min + world_aabb.max) * 0.5f;
        const glm::vec3 extents = (world_aabb.max - world_aabb.min) * 0.5f;

        for (const auto& plane : planes) {
            const float radius = glm::dot(extents, glm::abs(plane.normal));
            const float distance = glm::dot(plane.normal, center) + plane.distance;

            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }

    bool Frustum::intersects(const AABB& local_bounds, const glm::mat4& transform) const {
        const glm::vec3 center = (local_bounds.min + local_bounds.max) * 0.5f;
        const glm::vec3 extents = (local_bounds.max - local_bounds.min) * 0.5f;

        const glm::vec3 world_center = glm::vec3(transform * glm::vec4(center, 1.0f));

        const glm::mat3 abs_matrix = glm::mat3(
            glm::abs(glm::vec3(transform[0])),
            glm::abs(glm::vec3(transform[1])),
            glm::abs(glm::vec3(transform[2]))
        );
        const glm::vec3 world_extents = abs_matrix * extents;

        for (const auto& plane : planes) {
            const float radius = glm::dot(world_extents, glm::abs(plane.normal));
            const float distance = glm::dot(plane.normal, world_center) + plane.distance;

            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }
}
