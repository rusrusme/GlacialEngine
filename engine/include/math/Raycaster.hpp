//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <optional>
#include <glm/vec2.hpp>

#include "math/Ray.hpp"

namespace engine {
    class Camera;

    class Raycaster {
    public:
        /** @brief Casts a ray from the camera through the mouse position into the 3d world */
        static Ray cast_ray(glm::vec2 mouse_pos, int viewport_width, int viewport_height, const Camera& camera);

        /** @brief Intersects a ray with any surface supplying heightfield methods */
        template <typename HeightmapSurface>
        static std::optional<glm::vec3> intersect_heightmap(const Ray& ray, const HeightmapSurface& surface, const float max_distance = 1000.0f) {
            static constexpr float COARSE_STEP = 2.0f;
            static constexpr float ENTRY_STEP = 2.0f;
            float t_start = 0.0f;
            {
                bool found_entry = surface.is_in_bounds(
                    ray.origin.x + ray.direction.x * t_start,
                    ray.origin.z + ray.direction.z * t_start);
                if (!found_entry) {
                    for (float t = 0.0f; t < max_distance; t += ENTRY_STEP) {
                        glm::vec3 p = ray.origin + ray.direction * t;
                        if (surface.is_in_bounds(p.x, p.z)) {
                            t_start = t;
                            found_entry = true;
                            break;
                        }
                    }
                    if (!found_entry) return std::nullopt;
                }
            }

            float prev_t = t_start;
            bool was_above = true;
            {
                glm::vec3 p0 = ray.origin + ray.direction * t_start;
                was_above = p0.y > surface.get_height_at(p0.x, p0.z);
            }

            for (float t = t_start; t < max_distance; t += COARSE_STEP) {
                glm::vec3 p = ray.origin + ray.direction * t;
                if (!surface.is_in_bounds(p.x, p.z)) return std::nullopt;
                const float ground = surface.get_height_at(p.x, p.z);
                const bool above = p.y > ground;
                if (!above && was_above) {
                    // Binary search, 16 iterations (precise, no fine step needed)
                    float lo = prev_t, hi = t;
                    for (int i = 0; i < 16; ++i) {
                        float mid = (lo + hi) * 0.5f;
                        glm::vec3 mp = ray.origin + ray.direction * mid;
                        if (mp.y > surface.get_height_at(mp.x, mp.z)) lo = mid;
                        else hi = mid;
                    }
                    const float best_t = (lo + hi) * 0.5f;
                    glm::vec3 hit = ray.origin + ray.direction * best_t;
                    hit.y = surface.get_height_at(hit.x, hit.z);
                    return hit;
                }
                was_above = above;
                prev_t = t;
            }
            return std::nullopt;
        }
    };
}
