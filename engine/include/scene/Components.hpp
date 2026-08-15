//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include <utility>
#include <core/UUID.hpp>
#include <glm/glm.hpp>
#include <render/Camera.hpp>

namespace engine {
    struct LabelComponent {
        std::string tag;

        LabelComponent() = default;

        explicit LabelComponent(std::string t) : tag(std::move(t)) {}
    };

    struct IDComponent {
        UUID id;

        IDComponent() = default;
        IDComponent(const UUID& uuid) : id(uuid) {}
    };

    struct TransformComponent {
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f}; // Euler angles in degrees
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        TransformComponent() = default;

        TransformComponent(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
            : position(pos), rotation(rot), scale(scl) {}
    };

    struct MeshComponent {
        std::string mesh_path;

        MeshComponent() = default;

        explicit MeshComponent(std::string path) : mesh_path(std::move(path)) {}
    };

    struct CameraComponent {
        Camera camera;

        CameraComponent() = default;
        CameraComponent(const Camera& camera) : camera(camera) {}
    };
}
