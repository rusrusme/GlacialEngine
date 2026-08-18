//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include <utility>
#include <core/UUID.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <render/Camera.hpp>
#include <render/Mesh.hpp>

namespace engine {
    struct LabelComponent {
        std::string tag;

        LabelComponent() = default;
        LabelComponent(std::string t) : tag(std::move(t)) {}
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

        TransformComponent(const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& scl = glm::vec3(1.0f))
            : position(pos), rotation(rot), scale(scl) {}

        // Todo: Consider using quads to avoid gimbal lock
        [[nodiscard]] glm::mat4 get_matrix() const {
            glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);

            // Convert Euler degrees to radians for GLM
            mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            return glm::scale(mat, scale);
        }
    };

    struct MeshComponent {
        Mesh mesh;

        MeshComponent() = default;
        MeshComponent(const Mesh& m) : mesh(m) {}
    };

    struct CameraComponent {
        Camera camera;
        bool is_primary = true;

        CameraComponent() = default;
        CameraComponent(const Camera& camera) : camera(camera) {}
    };
}
