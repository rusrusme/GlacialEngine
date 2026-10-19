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
#include "asset/UUID.hpp"
#include "physics/AABB.hpp"
#include "render/StaticCamera.hpp"

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
        glm::vec3 rotation{0.0f, 0.0f, 0.0f}; // Euler-Winkel in Grad
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        TransformComponent() = default;

        TransformComponent(const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& scl = glm::vec3(1.0f))
            : position(pos), rotation(rot), scale(scl) {}

        [[nodiscard]] glm::quat get_orientation() const {
            return {glm::radians(rotation)};
        }

        [[nodiscard]] glm::mat4 get_matrix() const {
            if (is_dirty_ || position != last_position_ || rotation != last_rotation_ || scale != last_scale_) {
                glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
                mat *= glm::toMat4(get_orientation());
                cached_matrix_ = glm::scale(mat, scale);

                last_position_ = position;
                last_rotation_ = rotation;
                last_scale_ = scale;
                is_dirty_ = false;
            }

            return cached_matrix_;
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

        void set_dirty() const { is_dirty_ = true; }

    private:
        mutable glm::mat4 cached_matrix_{1.0f};
        mutable glm::vec3 last_position_{0.0f, 0.0f, 0.0f};
        mutable glm::vec3 last_rotation_{0.0f, 0.0f, 0.0f};
        mutable glm::vec3 last_scale_{1.0f, 1.0f, 1.0f};
        mutable bool is_dirty_{true};
    };

    struct DisabledComponent { /* Empty */ };

    struct MeshComponent {
        AssetHandle handle{0};

        MeshComponent() = default;
        MeshComponent(const AssetHandle h) : handle(h) {}
    };

    struct MaterialComponent {
        glm::vec4 color{1.0f};
        AssetHandle texture_handle{0};
        AssetHandle normal_handle{0};
        AssetHandle metallic_roughness_handle{0};

        float roughness = 0.5f;
        float metallic = 0.0f;

        MaterialComponent() = default;

        MaterialComponent(
            const glm::vec4 color,
            const AssetHandle texture = {0},
            const AssetHandle normal = {0},
            const AssetHandle pbr = {0}
        ) : color(color), texture_handle(texture), normal_handle(normal), metallic_roughness_handle(pbr) {}
    };

    struct CameraComponent {
        StaticCamera camera;
        bool is_primary = true;

        CameraComponent() = default;
        CameraComponent(const StaticCamera& camera) : camera(camera) {}
    };

    struct LODLevel {
        UUID mesh_handle;
        float max_distance{};
    };

    struct LODComponent {
        std::vector<LODLevel> levels;
        float max_draw_distance = 500.0f;
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

    // ----------------------------
    // Audio
    // ----------------------------
    enum class AudioTriggerCondition : uint8_t {
        PlayOnAwake,
        OnCollisionEnter,
        OnTriggerEnter,
        Manual
    };

    struct AudioSourceComponent {
        AssetHandle sound_handle{0};
        std::string filepath{};

        float volume = 1.0f;
        float pitch = 1.0f;
        bool loop = false;
        bool is_spatial = true; // true = 3D Spatial Audio, false = 2D

        float min_distance = 1.0f;
        float max_distance = 100.0f;

        AudioTriggerCondition trigger_condition = AudioTriggerCondition::PlayOnAwake;

        // Runtime state
        bool is_playing = false;

        // Runtime handle managed by AudioLayer
        void* runtime_sound_ptr = nullptr;

        AudioSourceComponent() = default;
        explicit AudioSourceComponent(const AssetHandle handle) : sound_handle(handle) {}
    };

    struct AudioListenerComponent {
        bool is_primary = true;

        AudioListenerComponent() = default;
    };
}
