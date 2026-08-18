//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "scene/Scene.hpp"

#include <core/UUID.hpp>
#include <scene/ecs/Entity.hpp>

namespace engine {
    Scene::Scene() = default;

    Scene::~Scene() = default;

    Entity Scene::create_entity(const std::string& name) {
        Entity entity = {registry_.create(), this};
        // ----------------------------------------
        // Metadata
        // ----------------------------------------
        entity.add_component<IDComponent>(UUID());
        auto& tag = entity.add_component<LabelComponent>();
        tag.tag = name.empty() ? "Entity" : name;
        // ----------------------------------------
        entity.add_component<TransformComponent>();
        return entity;
    }

    void Scene::destroy_entity(const Entity entity) {
        registry_.destroy(entity);
    }

    void Scene::on_init(Renderer& renderer) {
    }

    void Scene::on_shutdown() {
    }

    void Scene::on_update(float delta_time) {
    }

    void Scene::on_render(Renderer& renderer, Camera& camera, const float delta_time) {
        renderer.begin_scene(camera);

        // Rendering logic
        auto view = registry_.view<TransformComponent, MeshComponent>();
        for (auto entity : view) {
            auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);

            glm::vec4 color{1.0f};
            glm::mat4 transform_matrix = transform.get_matrix();

            if (registry_.all_of<MaterialComponent>(entity)) {
                color = registry_.get<MaterialComponent>(entity).color;
            }

            renderer.draw_mesh(mesh.mesh, transform_matrix, color);

        }

        renderer.end_scene(delta_time);
    }
}
