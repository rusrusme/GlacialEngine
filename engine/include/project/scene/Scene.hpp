//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <entt/entt.hpp>

#include "project/scene/ecs/Components.hpp"

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

    //////////////////////////////////
    // Compile-time System Concepts
    template<typename T>
    concept HasValidOnUpdate = requires(Scene& s, float dt) {
        { T::on_update(s, dt) } -> std::same_as<void>;
    };

    template<typename T>
    concept HasValidOnEvent = requires(Scene& s, Event& e) {
        { T::on_event(s, e) } -> std::same_as<void>;
    };

    // Diagnostics for common mistakes
    template<typename T>
    concept HasNonStaticOnUpdate = requires(T& instance, Scene& s, float dt) {
        { instance.on_update(s, dt) } -> std::same_as<void>;
    } && !HasValidOnUpdate<T>;

    template<typename T>
    concept HasNonStaticOnEvent = requires(T& instance, Scene& s, Event& e) {
        { instance.on_event(s, e) } -> std::same_as<void>;
    } && !HasValidOnEvent<T>;

    template<typename T>
    concept HasWrongUpdateSignature = (
        requires(float dt) { T::on_update(dt); } ||
        requires(Scene& s) { T::on_update(s); }
    ) && !HasValidOnUpdate<T>;
    //////////////////////////////////

    struct SceneSystemEntry {
        void(*on_update)(Scene&, float) = nullptr;
        void(*on_event)(Scene&, Event&) = nullptr;
    };

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

        /////////////////////////////////////////////////
        // Systems
        // Class-based registration: scene->add_system<MySystem>();
        template<typename T>
        void add_system() {
            static_assert(!HasNonStaticOnUpdate<T>,
                "[Scene::add_system] T::on_update must be declared 'static' -> static void on_update(Scene&, float)");

            static_assert(!HasNonStaticOnEvent<T>,
                "[Scene::add_system] T::on_event must be declared 'static' -> static void on_event(Scene&, Event&)");

            // Check for signature mistakes (e.g., forgot Scene&)
            static_assert(!HasWrongUpdateSignature<T>,
                "[Scene::add_system] Invalid parameter list in T::on_update. Expected signature: static void on_update(Scene&, float)");

            // Must implement at least one hook
            constexpr bool has_update = HasValidOnUpdate<T>;
            constexpr bool has_event  = HasValidOnEvent<T>;

            static_assert(has_update || has_event,
                "[Scene::add_system] Type does not qualify as an engine system. "
                "Must provide at least 'static void on_update(Scene&, float)' or 'static void on_event(Scene&, Event&)'");

            SceneSystemEntry entry{};

            if constexpr (has_update) {
                entry.on_update = &T::on_update;
            }

            if constexpr (has_event) {
                entry.on_event = &T::on_event;
            }

            systems_.push_back(entry);
        }

        // Convenience: scene->add_system(MySystem{});
        template<typename T>
        void add_system(T&&) {
            add_system<std::decay_t<T>>();
        }

        // Direct function-pointer registration for standalone free functions
        void add_system(void(*on_update)(Scene&, float), void(*on_event)(Scene&, Event&) = nullptr) {
            systems_.push_back({.on_update = on_update, .on_event = on_event});
        }

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
        std::vector<SceneSystemEntry> systems_;
    };
}
