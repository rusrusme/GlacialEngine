//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/Scene.hpp"

#include <utility>

#include "asset/AssetManager.hpp"
#include "asset/UUID.hpp"
#include "core/Log.hpp"
#include "core/SystemContext.hpp"
#include "physics/CollisionSystem.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "render/Camera.hpp"
#include "render/SceneRenderer.hpp"
#include "render/mesh/Mesh.hpp"
#include "util/Instrumentor.hpp"

namespace engine {
    Scene::Scene() = default;

    Scene::Scene(std::string name) : name_(std::move(name)) {}

    Scene::~Scene() = default;

    std::shared_ptr<Scene> Scene::copy(const std::shared_ptr<Scene>& source) {
        if (!source) return nullptr;

        auto destination = std::make_shared<Scene>();
        auto& final_registry = destination->get_registry();
        auto& source_registry = source->get_registry();

        std::unordered_map<entt::entity, entt::entity> entity_map;
        for (auto entity : source_registry.view<entt::entity>()) {
            entity_map[entity] = final_registry.create();
        }
        ComponentRegistry::copy_all_components(source_registry, final_registry, entity_map);

        CORE_TRACE("Copied scene with {0} entities", entity_map.size());
        return destination;
    }

    Entity Scene::create_entity(const std::string& name) {
        return create_entity_with_uuid(UUID(), name);
    }

    Entity Scene::create_entity_with_uuid(UUID uuid, const std::string& name) {
        Entity entity = {registry_.create(), this};
        entity.add_component<IDComponent>(uuid);

        auto& tag = entity.add_component<LabelComponent>();
        tag.tag = name.empty() ? "Entity" : name;

        entity.add_component<TransformComponent>();
        CORE_TRACE("Created entity - {0} [{1}]", tag.tag, static_cast<uint64_t>(uuid));
        return entity;
    }

    Entity Scene::duplicate_entity(Entity source) {
        std::string new_name = source.has_component<LabelComponent>() ? source.get_component<LabelComponent>().tag : "Entity";
        Entity destination = create_entity(new_name);

        const auto source_handle = source;
        const auto destination_handle = destination;

        for (auto&& [type_id, storage] : registry_.storage()) {
            if (type_id == entt::type_hash<IDComponent>::value() ||
                type_id == entt::type_hash<LabelComponent>::value()) {
                continue;
            }
            if (storage.contains(source_handle)) {
                if (storage.contains(destination_handle)) {
                    storage.erase(destination_handle);
                }
                storage.push(destination_handle, storage.value(source_handle));
            }
        }
        CORE_TRACE("Duplicated entity '{0}' -> new entity [{1}]", new_name, static_cast<uint64_t>(destination.get_component<IDComponent>().id));
        return destination;
    }

    void Scene::destroy_entity(Entity entity) {
        if (registry_.valid(entity)) {
            std::string name = entity.has_component<LabelComponent>() ? entity.get_component<LabelComponent>().tag : "Entity";
            uint64_t uuid = entity.has_component<IDComponent>() ? static_cast<uint64_t>(entity.get_component<IDComponent>().id) : 0;

            CORE_TRACE("Destroyed entity '{0}' [{1}]", name, uuid);
            registry_.destroy(entity);
        }
    }

    void Scene::on_init() {
        collision_system_ = std::make_unique<CollisionSystem>(10.0f);
        CORE_TRACE("Scene initialized");
    }

    void Scene::on_shutdown() {
        collision_system_.reset();
        CORE_TRACE("Scene shutdown");
    }

    void Scene::on_update(float delta_time) {
        PROFILE_FUNCTION();
        collision_system_->update_broadphase(*this);
    }

    void Scene::on_render(const SystemContext& context, float delta_time, const Camera* override_camera) {
        PROFILE_FUNCTION();

        const Camera* active_camera = override_camera;
        Camera scene_camera;

        if (!active_camera) {
            const auto camera_view = registry_.view<TransformComponent, CameraComponent>(entt::exclude<DisabledComponent>);
            for (const auto& [entity, transform, camera_comp] : camera_view.each()) {
                if (!camera_comp.is_primary) continue;

                scene_camera = camera_comp.camera;
                scene_camera.set_position(transform.position);
                scene_camera.set_target(transform.position + transform.get_forward());
                active_camera = &scene_camera;
                break;
            }
        }

        if (!active_camera) return;

        context.renderer.begin_scene(active_camera->gpu_data());

        const glm::vec3 camera_pos = active_camera->position();

        const auto mesh_view = registry_.view<TransformComponent, MeshComponent>(entt::exclude<DisabledComponent>);
        for (const auto& [entity, transform, mesh_comp] : mesh_view.each()) {
            AssetHandle target_mesh_handle = mesh_comp.handle;

            const float max_scale = std::max({transform.scale.x, transform.scale.y, transform.scale.z});
            const float distance_to_camera = glm::distance(transform.position, camera_pos) / std::max(max_scale, 0.001f);

            if (registry_.all_of<LODComponent>(entity)) {
                const auto& [levels, max_draw_distance] = registry_.get<LODComponent>(entity);
                if (distance_to_camera > max_draw_distance) {
                    continue;
                }

                for (const auto& level : levels) {
                    if (distance_to_camera > level.max_distance) {
                        target_mesh_handle = level.mesh_handle;
                    }
                    else {
                        break;
                    }
                }
            }

            Mesh* mesh_asset = context.assets.get_asset<Mesh>(target_mesh_handle).get();
            if (!mesh_asset) {
                CORE_WARN("Failed to retrieve mesh asset handle {0} for entity '{1}'",
                          static_cast<uint64_t>(target_mesh_handle),
                          registry_.all_of<LabelComponent>(entity) ? registry_.get<LabelComponent>(entity).tag : "Entity");
                continue;
            }

            const glm::mat4 model_matrix = transform.get_matrix();

            // Bounding Sphere in World-Space für GPU Compute Culling berechnen
            const AABB& local_bounds = mesh_asset->bounds();
            const glm::vec3 bounds_center_local = (local_bounds.min + local_bounds.max) * 0.5f;
            const glm::vec3 bounds_extent_local = (local_bounds.max - local_bounds.min) * 0.5f;

            const auto world_center = glm::vec3(model_matrix * glm::vec4(bounds_center_local, 1.0f));
            const float world_radius = glm::length(bounds_extent_local * transform.scale);

            GPUInstanceData instance_data{};
            instance_data.model_matrix = model_matrix;
            instance_data.bounding_center_radius = glm::vec4(world_center, world_radius);
            instance_data.bounding_box_min = glm::vec4(local_bounds.min, 1.0f);
            instance_data.bounding_box_max = glm::vec4(local_bounds.max, 1.0f);

            // Submeshes an den SceneRenderer übergeben
            for (const Submesh& submesh : mesh_asset->submeshes()) {
                instance_data.mesh_id = submesh.mesh_id;
                instance_data.material_id = submesh.default_material_id;

                context.renderer.submit_instance(instance_data, submesh.mesh_id);
            }
        }
        context.renderer.end_scene();
    }

    void Scene::clear() {
        size_t entity_count = registry_.storage<entt::entity>().size();

        registry_.clear();
        registry_.compact();

        CORE_TRACE("Scene cleared ({} entities removed)", entity_count);
    }

    Entity Scene::create_cube(const std::string& name, const glm::vec4& color) {
        Entity entity = create_entity(name);

        entity.add_component<MeshComponent>(AssetHandle(3)); // Todo: Broken Asset handle
        entity.add_component<MaterialComponent>(color);
        return entity;
    }

    Entity Scene::create_point_light(const std::string& name, const glm::vec3& position, const glm::vec3& color, float intensity, float radius) {
        Entity light = create_entity(name);
        auto& transform = light.get_component<TransformComponent>();
        transform.position = position;

        light.add_component<PointLightComponent>(color, intensity, radius);
        return light;
    }
}
