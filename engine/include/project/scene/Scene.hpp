//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <entt/entt.hpp>

#include "project/scene/ecs/Components.hpp"

namespace engine {
    class Camera;
    class CollisionSystem;
    class Entity;
    struct SystemContext;

    class Scene {
    public:
        Scene();
        explicit Scene(std::string name);
        ~Scene();

        static std::shared_ptr<Scene> copy(const std::shared_ptr<Scene>& source);

        Entity create_entity(const std::string& name = "Entity");
        Entity create_entity_with_uuid(UUID uuid, const std::string& name = "Entity");
        Entity duplicate_entity(Entity entity);
        void destroy_entity(Entity entity);

        void on_init();
        void on_shutdown();
        void on_update(float delta_time);
        void on_render(const SystemContext& context, float delta_time, const Camera* override_camera = nullptr);

        void clear();

        template<typename... Components, typename... Args>
        auto view(Args&&... args) { return registry_.view<Components...>(std::forward<Args>(args)...); }

        template<typename... Components>
        auto clear_component() { return registry_.clear<Components...>(); }

        Entity create_cube(const std::string& name, const glm::vec4& color = glm::vec4(1.0f));
        Entity create_point_light(const std::string& name, const glm::vec3& position, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f, float radius = 10.0f);

        [[nodiscard]] CollisionSystem& get_collision_system() { return *collision_system_; }

        [[nodiscard]] const std::string& get_name() const { return name_; }
        void set_name(const std::string& name) { name_ = name; }

        [[nodiscard]] const std::string& get_path() const { return path_; }
        void set_path(const std::string& path) { path_ = path; }

    private:
        friend class Entity;

        [[nodiscard]] entt::registry& get_registry() { return registry_; }

        std::string name_{"Untitled"};
        std::string path_;

        entt::registry registry_;
        std::unique_ptr<CollisionSystem> collision_system_ = nullptr;
    };
}
