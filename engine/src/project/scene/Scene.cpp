//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/Scene.hpp"

#include <utility>

#include "asset/AssetManager.hpp"
#include "asset/UUID.hpp"
#include "core/Log.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "render/camera/CameraSystem.hpp"
#include "render/mesh/Mesh.hpp"
#include "script/ScriptRegistry.hpp"
#include "util/Instrumentor.hpp"

namespace engine {
    Scene::Scene() = default;

    Scene::Scene(std::string name) : name_(std::move(name)) {}

    Scene::~Scene() = default;

    std::shared_ptr<Scene> Scene::copy(const std::shared_ptr<Scene>& source) {
        if (!source) return nullptr;

        auto destination = std::make_shared<Scene>();
        auto& final_registry = destination->registry();
        auto& source_registry = source->registry();

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
        ScriptRegistry::bind_scene(*this);

        CORE_TRACE("Scene initialized");
    }

    void Scene::on_shutdown() {
        systems_.clear();

        CORE_TRACE("Scene shutdown");
    }

    void Scene::on_update(float delta_time) {
        PROFILE_FUNCTION();

        systems_.on_update(*this, delta_time);

        CameraSystem::on_update(*this, delta_time);
    }

    void Scene::on_event(Event& event) {
        PROFILE_FUNCTION();

        systems_.on_event(*this, event);

        // TODO Consider if should be here
        CameraSystem::on_event(*this, event);
    }

    void Scene::clear() {
        size_t entity_count = registry_.storage<entt::entity>().size();

        registry_.clear();
        registry_.compact();

        CORE_TRACE("Scene cleared ({} entities removed)", entity_count);
    }
}
