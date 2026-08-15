//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <entt/entt.hpp>
#include <render/Camera.hpp>
#include <render/Renderer.hpp>
#include <scene/Components.hpp>

namespace engine {
    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        Entity create_entity(const std::string& name = std::string());
        void destroy_entity(Entity entity);

        void on_init(Renderer& renderer);
        void on_shutdown();
        void on_update(float delta_time);
        void on_render(const Renderer& renderer);

    private:
        friend class Entity;

        entt::registry registry_;

        // Todo: Rethink architectural design (component vs member)
        std::unique_ptr<Camera> camera_ = nullptr;
    };
}
