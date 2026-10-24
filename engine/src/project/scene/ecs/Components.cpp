//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/ecs/Components.hpp"

namespace engine {
    TransformComponent::TransformComponent(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl) : position(pos), rotation(rot), scale(scl) {}

    glm::quat TransformComponent::get_orientation() const {
        return {glm::radians(rotation)};
    }

    glm::mat4 TransformComponent::get_matrix() const {
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

    glm::vec3 TransformComponent::get_forward() const {
        return glm::rotate(get_orientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 TransformComponent::get_up() const {
        return glm::rotate(get_orientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 TransformComponent::get_right() const {
        return glm::rotate(get_orientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    void TransformComponent::set_dirty() const { is_dirty_ = true; }

    CameraComponent::CameraComponent(float fov, float near_clip, float far_clip, bool is_primary)
        : is_primary(is_primary), native{fov, 16.0f / 9.0f, near_clip, far_clip} {}

    const glm::mat4& CameraComponent::view() const noexcept { return native.view(); }

    const glm::mat4& CameraComponent::view_projection() const noexcept { return native.view_projection(); }

    glm::mat4 CameraComponent::projection() const { return native.projection(); }

    void CameraComponent::set_aspect_ratio(float aspect) { native.set_aspect_ratio(aspect); }

    void CameraComponent::set_viewport_size(uint32_t width, uint32_t height) { native.set_viewport_size(width, height); }

    void CameraComponent::set_fov(float fov) { native.set_fov(fov); }

    void CameraComponent::set_near_clip(float near_clip) { native.set_near_clip(near_clip); }

    void CameraComponent::set_far_clip(float far_clip) { native.set_far_clip(far_clip); }
}
