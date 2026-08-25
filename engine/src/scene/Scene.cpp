//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "scene/Scene.hpp"

#include "asset/AssetManager.hpp"
#include "core/Log.hpp"
#include "core/UUID.hpp"
#include "physics/CollisionSystem.hpp"
#include "render/mesh/InstanceData.hpp"
#include "scene/ecs/Entity.hpp"

namespace engine {
    Scene::Scene() = default;

    Scene::~Scene() = default;

    Entity Scene::create_entity(const std::string& name) {
        return create_entity_with_uuid(UUID(), name);
    }

    Entity Scene::create_entity_with_uuid(UUID uuid, const std::string& name) {
        Entity entity = {registry_.create(), this};
        entity.add_component<IDComponent>(uuid);

        auto& tag = entity.add_component<LabelComponent>();
        tag.tag = name.empty() ? "Entity" : name;

        entity.add_component<TransformComponent>();
        CORE_INFO("Created entity - {0} [{1}]", name, static_cast<uint64_t>(uuid));
        return entity;
    }

    void Scene::destroy_entity(Entity entity) {
        CORE_INFO("Deleted entity - {0} [{1}]", entity.name(), static_cast<uint64_t>(entity.uuid()));
        registry_.destroy(entity);
    }

    void Scene::on_init(Renderer& renderer) {
        collision_system_ = std::make_unique<CollisionSystem>(10.0f);
    }

    void Scene::on_shutdown() {
        collision_system_.reset();
    }

    void Scene::on_update(float delta_time) {
        collision_system_->update_broadphase(*this);
    }

    void Scene::on_render(Renderer& renderer, Camera& camera, AssetManager& asset_manager, const float delta_time) {
        renderer.begin_scene(camera);
        renderer.set_blend(true);

        // ------------------
        // Directional lights
        // ------------------
        auto dir_light_view = registry_.view<TransformComponent, DirectionalLightComponent>();
        for (auto entity : dir_light_view) {
            auto [transform, light] = dir_light_view.get<TransformComponent, DirectionalLightComponent>(entity);

            renderer.add_directional_light({
                .direction = transform.get_forward(),
                .color     = light.color,
                .intensity = light.intensity
            });
        }
        // ------------------
        // Point lights
        // ------------------
        auto point_light_view = registry_.view<TransformComponent, PointLightComponent>();
        for (auto entity : point_light_view) {
            auto [transform, light] = point_light_view.get<TransformComponent, PointLightComponent>(entity);

            renderer.add_point_light({
                .position  = transform.position,
                .color     = light.color,
                .intensity = light.intensity,
                .radius    = light.radius
            });
        }
        // ------------------
        // Meshes
        // ------------------
        std::unordered_map<UUID, std::vector<InstanceData>> mesh_batches;

        auto mesh_view = registry_.view<TransformComponent, MeshComponent>();
        for (auto entity : mesh_view) {
            auto [transform, mesh_comp] = mesh_view.get<TransformComponent, MeshComponent>(entity);

            glm::vec4 color{1.0f};
            if (registry_.all_of<MaterialComponent>(entity)) {
                color = registry_.get<MaterialComponent>(entity).color;
            }

            mesh_batches[mesh_comp.handle].push_back(InstanceData{
                .transform = transform.get_matrix(),
                .color     = color
            });
        }
        for (const auto& [handle, instance_list] : mesh_batches) {
            if (std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle)) {
                renderer.draw_mesh_instanced(*mesh, instance_list);
            }
        }
        renderer.end_scene(delta_time);
    }
}
