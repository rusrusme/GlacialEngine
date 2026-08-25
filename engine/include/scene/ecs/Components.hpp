//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <utility>

#include "asset/AssetType.hpp"
#include "core/UUID.hpp"
#include "physics/AABB.hpp"
#include "render/Camera.hpp"

namespace engine {
    // ----------------------------
    // Data
    // ----------------------------
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

    // ----------------------------
    // Rendering
    // ----------------------------
    struct TransformComponent {
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f}; // Euler-Winkel in Grad (für Inspector/UI)
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        TransformComponent() = default;

        TransformComponent(const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& scl = glm::vec3(1.0f))
            : position(pos), rotation(rot), scale(scl) {}

        [[nodiscard]] glm::quat get_orientation() const {
            return {glm::radians(rotation)};
        }

        [[nodiscard]] glm::mat4 get_matrix() const {
            glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
            mat *= glm::toMat4(get_orientation());
            return glm::scale(mat, scale);
        }

        [[nodiscard]] glm::vec3 get_forward() const {
            return glm::rotate(get_orientation(), glm::vec3(0.0f, 0.0f, -1.0f));
        }

        [[nodiscard]] glm::vec3 get_up() const {
            return glm::rotate(get_orientation(), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        [[nodiscard]] glm::vec3 get_right() const {
            return glm::rotate(get_orientation(), glm::vec3(1.0f, 0.0f, 0.0f));
        }
    };

    struct ActiveComponent {
        bool active = true;
    };

    struct MeshComponent {
        AssetHandle handle{0};

        MeshComponent() = default;
        MeshComponent(const AssetHandle h) : handle(h) {}
    };

    struct MaterialComponent {
        glm::vec4 color{1.0f};

        MaterialComponent() = default;
        MaterialComponent(const glm::vec4 color) : color(color) {}
    };

    struct CameraComponent {
        Camera camera;
        bool is_primary = true;

        CameraComponent() = default;
        CameraComponent(const Camera& camera) : camera(camera) {}
    };

    // ----------------------------
    // Lighting
    // ----------------------------
    struct DirectionalLightComponent {
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity{1.0f};

        DirectionalLightComponent() = default;
        DirectionalLightComponent(const glm::vec3& color, float intensity = 1.0f) : color(color), intensity(intensity) {}
    };

    struct PointLightComponent {
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity{1.0f};
        float radius{1.0f};

        PointLightComponent() = default;

        PointLightComponent(const glm::vec3& color, const float intensity = 1.0f, const float radius = 10.0f)
            : color(color), intensity(intensity), radius(radius) {}
    };

    // ----------------------------
    // Physics
    // ----------------------------
    enum class BodyType {
        STATIC,
        KINEMATIC,
        DYNAMIC
    };

    struct RigidBodyComponent {
        BodyType type = BodyType::STATIC;
        glm::vec3 velocity{0.0f};
    };

    struct BoxColliderComponent {
        glm::vec3 center{0.0f};
        glm::vec3 size{1.0f};
        bool is_trigger = false;
        bool show_hitbox = false;

        [[nodiscard]] AABB get_world_aabb(const glm::vec3& position, const glm::vec3& scale) const {
            const glm::vec3 half_size = (size * scale) * 0.5f;
            const glm::vec3 world_center = position + (center * scale);
            return {world_center - half_size, world_center + half_size};
        }
    };

    // UI
    struct UIComponent {
        std::string rml_filepath;
        bool is_screen_space = true;
        bool is_visible = true;
    };
}
