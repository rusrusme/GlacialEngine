//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <entt/entt.hpp>

#include "project/scene/ecs/Components.hpp"
#include "project/scene/ecs/SystemDispatcher.hpp"

namespace engine {
    template<typename... Components>
    using exclude_types = entt::exclude_t<Components...>;

    template<typename... Components>
    inline constexpr auto exclude = entt::exclude<Components...>;

    class Camera;
    class CameraSystem;
    class CollisionSystem;
    class FlyCameraControllerSystem;
    class Entity;
    class Event;
    struct SystemContext;
    class Scene;

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
        void on_event(Event& event);

        void clear();

        template<typename... Components, typename... Excluded>
        auto view(entt::exclude_t<Excluded...> excluded) {
            return registry_.view<Components...>(excluded);
        }

        template<typename... Components>
        auto view() {
            return registry_.view<Components...>();
        }

        template<typename... Components>
        auto clear_component() { return registry_.clear<Components...>(); }

        [[nodiscard]] SystemDispatcher& systems() { return systems_; }

        [[nodiscard]] const std::string& name() const { return name_; }
        void set_name(const std::string& name) { name_ = name; }

        [[nodiscard]] const std::string& path() const { return path_; }
        void set_path(const std::string& path) { path_ = path; }

    private:
        friend class Entity;

        [[nodiscard]] entt::registry& registry() { return registry_; }

        std::string name_{"Untitled"};
        std::string path_;

        entt::registry registry_;
        SystemDispatcher systems_;
    };
}
