//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

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
        void destroy_entity(Entity entity);

        void on_init(Renderer& renderer);
        void on_shutdown();
        void on_update(float delta_time);
        void on_render(Renderer& renderer, Camera& camera, float delta_time);

        entt::registry& get_registry() { return registry_; }

    private:
        friend class Entity;

        entt::registry registry_;
    };
}
