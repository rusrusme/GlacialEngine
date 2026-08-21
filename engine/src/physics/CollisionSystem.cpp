//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "physics/CollisionSystem.hpp"

namespace engine {
    CollisionSystem::CollisionSystem(float cell_size) : spatial_grid_(cell_size) {}

    void CollisionSystem::update_broadphase(Scene& scene) {
        spatial_grid_.clear();

        auto view = scene.view<TransformComponent, BoxColliderComponent>();
        for (auto entity_handle : view) {
            auto [transform, collider] = view.get<TransformComponent, BoxColliderComponent>(entity_handle);
            AABB world_aabb = collider.get_world_aabb(transform.position, transform.scale);
            spatial_grid_.insert(entity_handle, world_aabb);
        }
    }

    std::optional<RaycastHit> CollisionSystem::raycast(Scene& scene, const Ray& ray, float max_distance) {
        std::optional<RaycastHit> closest_hit;
        float min_dist = max_distance;

        auto view = scene.view<TransformComponent, BoxColliderComponent>();
        for (auto entity_handle : view) {
            auto [transform, collider] = view.get<TransformComponent, BoxColliderComponent>(entity_handle);
            AABB world_aabb = collider.get_world_aabb(transform.position, transform.scale);

            float t_hit = 0.0f;
            if (world_aabb.intersects_ray(ray, t_hit)) {
                if (t_hit >= 0.0f && t_hit < min_dist) {
                    min_dist = t_hit;
                    closest_hit = RaycastHit{
                        .entity   = Entity{entity_handle, &scene},
                        .point    = ray.at(t_hit),
                        .distance = t_hit
                    };
                }
            }
        }
        return closest_hit;
    }

    bool CollisionSystem::intersects_any(Scene& scene, const AABB& target_aabb) const {
        auto candidates = spatial_grid_.query_range(target_aabb);

        for (auto entity_handle : candidates) {
            Entity entity{entity_handle, &scene};
            if (!entity.has_component<BoxColliderComponent>() || !entity.has_component<TransformComponent>()) continue;

            const auto& transform = entity.get_component<TransformComponent>();
            const auto& collider = entity.get_component<BoxColliderComponent>();

            AABB world_aabb = collider.get_world_aabb(transform.position, transform.scale);
            if (target_aabb.intersects_aabb(world_aabb)) {
                return true;
            }
        }
        return false;
    }

    std::vector<Entity> CollisionSystem::query_aabb(Scene& scene, const AABB& target_aabb) const {
        std::vector<Entity> results;

        auto candidate_handles = spatial_grid_.query_range(target_aabb);

        std::ranges::sort(candidate_handles);
        candidate_handles.erase(std::ranges::unique(candidate_handles).begin(), candidate_handles.end());

        for (auto entity_handle : candidate_handles) {
            Entity entity{entity_handle, &scene};
            if (!entity.has_component<TransformComponent>() || !entity.has_component<BoxColliderComponent>()) {
                continue;
            }
            const auto& transform = entity.get_component<TransformComponent>();
            const auto& collider = entity.get_component<BoxColliderComponent>();

            AABB world_aabb = collider.get_world_aabb(transform.position, transform.scale);
            if (target_aabb.intersects_aabb(world_aabb)) {
                results.push_back(entity);
            }
        }
        return results;
    }
}
