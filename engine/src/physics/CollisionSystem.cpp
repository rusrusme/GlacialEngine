//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "physics/CollisionSystem.hpp"

#include "asset/AssetManager.hpp"

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

    std::optional<RaycastHit> CollisionSystem::raycast(Scene& scene, const Ray& ray, float max_distance, AssetManager* asset_manager) {
        std::optional<RaycastHit> closest_hit;
        float min_dist = max_distance;

        auto view = scene.view<TransformComponent>();
        for (auto entity_handle : view) {
            Entity entity{entity_handle, &scene};
            const auto& transform = entity.get_component<TransformComponent>();
            AABB world_aabb;

            if (entity.has_component<BoxColliderComponent>()) {
                const auto& collider = entity.get_component<BoxColliderComponent>();
                world_aabb = collider.get_world_aabb(transform.position, transform.scale);
            }
            else if (asset_manager && entity.has_component<MeshComponent>()) {
                const auto& mesh_comp = entity.get_component<MeshComponent>();
                if (auto mesh = asset_manager->get_asset<Mesh>(mesh_comp.handle)) {
                    glm::vec3 min_pt = transform.position + mesh->local_bounds.min * transform.scale;
                    glm::vec3 max_pt = transform.position + mesh->local_bounds.max * transform.scale;
                    world_aabb = AABB(glm::min(min_pt, max_pt), glm::max(min_pt, max_pt));
                }
                else {
                    constexpr float half_size = 0.5f;
                    world_aabb = AABB(transform.position - glm::vec3(half_size), transform.position + glm::vec3(half_size));
                }
            }
            else {
                constexpr float half_size = 0.5f;
                world_aabb = AABB(transform.position - glm::vec3(half_size), transform.position + glm::vec3(half_size));
            }

            float t_hit = 0.0f;
            if (world_aabb.intersects_ray(ray, t_hit)) {
                if (t_hit >= 0.0f && t_hit < min_dist) {
                    min_dist = t_hit;
                    closest_hit = RaycastHit{
                        .entity   = entity,
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
