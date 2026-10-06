//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <entt/entt.hpp>

#include "project/scene/ecs/Components.hpp"
#include "render/legacy/Renderer.hpp"

namespace engine {
    class AssetManager;
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
        void on_render(Renderer& renderer, Camera& camera, AssetManager& asset_manager, float delta_time);

        void clear(const SystemContext& context);

        template<typename... Components>
        auto view() { return registry_.view<Components...>(); }

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
        std::string name_{"Untitled"};
        std::string path_;

        friend class Entity;

        [[nodiscard]] entt::registry& get_registry() { return registry_; }

        entt::registry registry_;
        std::unique_ptr<CollisionSystem> collision_system_ = nullptr;

        // Todo: Combine into RenderCommand package
        std::unordered_map<UUID, std::vector<InstanceData>> mesh_batches_; // camera-visible batches
        std::unordered_map<UUID, std::vector<InstanceData>> shadow_batches_;
    };
}
