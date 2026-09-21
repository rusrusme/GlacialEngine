//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <entt/entt.hpp>

#include "render/Renderer.hpp"
#include "scene/ecs/Components.hpp"

namespace engine {
    class AssetManager;
    class Camera;
    class CollisionSystem;
    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        Entity create_entity(const std::string& name = "Entity");
        Entity create_entity_with_uuid(UUID uuid, const std::string& name = "Entity");
        Entity duplicate_entity(Entity entity);
        void destroy_entity(Entity entity);

        void on_init(Renderer& renderer);
        void on_shutdown();
        void on_update(float delta_time);
        void on_render(Renderer& renderer, Camera& camera, AssetManager& asset_manager, float delta_time);

        template<typename... Components>
        auto view() { return registry_.view<Components...>(); }

        template<typename... Components>
        auto clear() { return registry_.clear<Components...>(); }

        [[deprecated]] entt::registry& get_registry() { return registry_; }
        [[nodiscard]] CollisionSystem& get_collision_system() { return *collision_system_; }

    private:
        friend class Entity;

        entt::registry registry_;
        std::unique_ptr<CollisionSystem> collision_system_ = nullptr;

        // Todo: Combine into RenderCommand package
        std::unordered_map<UUID, std::vector<InstanceData>> mesh_batches_;
    };
}
