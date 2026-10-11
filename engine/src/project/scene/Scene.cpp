//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/Scene.hpp"

#include <utility>

#include "asset/AssetManager.hpp"
#include "core/Log.hpp"
#include "core/SystemContext.hpp"
#include "asset/UUID.hpp"
#include "physics/CollisionSystem.hpp"
#include "project/scene/ecs/Entity.hpp"
// #include "render/legacy/mesh/InstanceData.hpp"
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

    void Scene::on_render(/*Renderer& renderer,*/ Camera& camera, AssetManager& asset_manager, const float delta_time) {
    //     PROFILE_FUNCTION();
    //
    //     Renderer::set_blend(true);
    //
    //     // Lights
    //     PROFILE_START("Scene::on_render - Lights Processing");
    //     const auto dir_light_view = registry_.view<TransformComponent, DirectionalLightComponent>(entt::exclude<DisabledComponent>);
    //     for (const auto& [entity, transform, light] : dir_light_view.each()) {
    //         renderer.add_directional_light({
    //             .direction = transform.get_forward(),
    //             .color     = light.color,
    //             .intensity = light.intensity
    //         });
    //     }
    //
    //     const auto point_light_view = registry_.view<TransformComponent, PointLightComponent>(entt::exclude<DisabledComponent>);
    //     for (const auto& [entity, transform, light] : point_light_view.each()) {
    //         renderer.add_point_light({
    //             .position  = transform.position,
    //             .color     = light.color,
    //             .intensity = light.intensity,
    //             .radius    = light.radius
    //         });
    //     }
    //     PROFILE_END();
    //
    //     // Meshes
    //     PROFILE_START("Scene::on_render - Batch Clearing");
    //     mesh_batches_.clear();
    //     shadow_batches_.clear();
    //     PROFILE_END();
    //
    //     const glm::vec3 camera_pos = camera.get_position();
    //
    //     PROFILE_START("Scene::on_render - Batch Loop");
    //     const auto mesh_group = registry_.group<TransformComponent, MeshComponent>(entt::get<>, entt::exclude<DisabledComponent>);
    //     for (const auto& [entity, transform, mesh_comp] : mesh_group.each()) {
    //         UUID target_mesh_handle = mesh_comp.handle;
    //
    //         const float max_scale = std::max({transform.scale.x, transform.scale.y, transform.scale.z});
    //         const float distance_to_camera = glm::distance(transform.position, camera_pos) / std::max(max_scale, 0.001f);
    //
    //         if (registry_.all_of<LODComponent>(entity)) {
    //             const auto& [levels, max_draw_distance] = registry_.get<LODComponent>(entity);
    //             if (distance_to_camera > max_draw_distance) continue;
    //
    //             for (const auto& [mesh_handle, max_distance] : levels) {
    //                 if (distance_to_camera > max_distance) {
    //                     target_mesh_handle = mesh_handle;
    //                 }
    //                 else {
    //                     break;
    //                 }
    //             }
    //         }
    //
    //         if (const auto mesh = asset_manager.get_asset<Mesh>(target_mesh_handle)) {
    //             const glm::mat4 model_matrix = transform.get_matrix();
    //
    //             // Shadow batches
    //             shadow_batches_[target_mesh_handle].push_back(InstanceData{
    //                 .transform = model_matrix,
    //                 .color     = glm::vec4{1.0f}
    //             });
    //
    //             // Camera batches (mesh batches)
    //             glm::vec4 color{1.0f};
    //             float roughness = 0.5f;
    //             float metallic = 0.0f;
    //             if (registry_.all_of<MaterialComponent>(entity)) {
    //                 const auto& mat = registry_.get<MaterialComponent>(entity);
    //                 color = mat.color;
    //                 roughness = mat.roughness;
    //                 metallic = mat.metallic;
    //             }
    //             mesh_batches_[target_mesh_handle].push_back(InstanceData{
    //                 .transform = model_matrix,
    //                 .color     = color,
    //                 .pbr       = glm::vec2(roughness, metallic)
    //             });
    //         }
    //         else {
    //             CORE_WARN("Failed to retrieve mesh asset handle {0} for entity '{1}'",
    //                       static_cast<uint64_t>(target_mesh_handle),
    //                       registry_.all_of<LabelComponent>(entity) ? registry_.get<LabelComponent>(entity).tag : "Entity");
    //         }
    //     }
    //     PROFILE_END();
    //     // ------------------
    //     // Render passes
    //     // ------------------
    //     PROFILE_START("Scene::on_render - Pass Submissions");
    //     // renderer.render_occlusion_pass(camera, asset_manager, mesh_batches_);
    //     renderer.render_shadow_pass(camera, asset_manager, shadow_batches_);
    //     renderer.render_point_shadow_pass(asset_manager, shadow_batches_);
    //     renderer.render_mesh_batches(asset_manager, mesh_batches_);
    //     PROFILE_END();
    }
    //
    void Scene::clear(const SystemContext& context) {
    //     size_t entity_count = registry_.storage<entt::entity>().size();
    //
    //     registry_.clear();
    //     registry_.compact();
    //     mesh_batches_.clear();
    //
    //     context.renderer.reset_buffers();
    //
    //     CORE_TRACE("Scene cleared ({0} entities removed)", entity_count);
    }

    Entity Scene::create_cube(const std::string& name, const glm::vec4& color) {
        Entity entity = create_entity(name);

        entity.add_component<MeshComponent>(AssetHandle(3));
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
