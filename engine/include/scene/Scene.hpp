//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <asset/AssetManager.hpp>
#include <entt/entt.hpp>
#include <render/Camera.hpp>
#include <render/Renderer.hpp>
#include <scene/ecs/Components.hpp>

namespace engine {
    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        Entity create_entity(const std::string& name = "Entity");
        Entity create_entity_with_uuid(UUID uuid, const std::string& name = "Entity");
        void destroy_entity(Entity entity);

        void on_init(Renderer& renderer);
        void on_shutdown();
        void on_update(float delta_time);
        void on_render(Renderer& renderer, Camera& camera, AssetManager& asset_manager, float delta_time);

        template <typename... Components>
        auto view() { return registry_.view<Components...>(); }

        template <typename... Components>
        auto clear() { return registry_.clear<Components...>(); }

        [[deprecated]] entt::registry& get_registry() { return registry_; }

    private:
        friend class Entity;

        entt::registry registry_;
    };
}
