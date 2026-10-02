//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <random>

#include "project/scene/Scene.hpp"
#include "project/scene/ecs/Entity.hpp"

namespace editor {
    static void generate_large_test_scene(const engine::SystemContext& context, const std::shared_ptr<engine::Scene>& scene, size_t count = 10000) {
        if (!scene) return;

        // Clear existing scene before spawning
        scene->clear(context);

        std::mt19937 rng(1337); // Fixed seed for reproducible generation
        std::uniform_real_distribution<float> pos_dist(-150.0f, 150.0f);
        std::uniform_real_distribution<float> scale_dist(0.5f, 2.0f);
        std::uniform_real_distribution<float> rot_dist(0.0f, 360.0f);
        std::uniform_real_distribution<float> color_dist(0.1f, 1.0f);
        std::uniform_int_distribution<uint64_t> mesh_dist(1, 5); // Primitive handles: 1=plane, 2=circle, 3=cube, 4=sphere, 5=cylinder

        for (size_t i = 0; i < count; ++i) {
            std::string name = "Entity_" + std::to_string(i);
            engine::Entity entity = scene->create_entity(name);

            // Set random transform
            auto& transform = entity.get_component<engine::TransformComponent>();
            transform.position = glm::vec3(pos_dist(rng), pos_dist(rng) * 0.2f, pos_dist(rng)); // Keep height slightly flatter
            transform.rotation = glm::vec3(rot_dist(rng), rot_dist(rng), rot_dist(rng));
            transform.scale = glm::vec3(scale_dist(rng));

            // Assign primitive mesh handle
            entity.add_component<engine::MeshComponent>(engine::AssetHandle(mesh_dist(rng)));

            // Assign random material color
            glm::vec4 color(color_dist(rng), color_dist(rng), color_dist(rng), 1.0f);
            entity.add_component<engine::MaterialComponent>(color);
        }

        // Add a few point lights across the scene to test lighting performance
        for (int i = 0; i < 20; ++i) {
            glm::vec3 light_pos(pos_dist(rng), 10.0f, pos_dist(rng));
            glm::vec3 light_color(color_dist(rng), color_dist(rng), color_dist(rng));
            scene->create_point_light("Light_" + std::to_string(i), light_pos, light_color, 2.0f, 30.0f);
        }
    }
}
