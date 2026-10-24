//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/ecs/Components.hpp"

namespace engine {
    TransformComponent::TransformComponent(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
        : position(pos), rotation(rot), scale(scl) {}

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
        : fov(fov), near_clip(near_clip), far_clip(far_clip), is_primary(is_primary) {}

    void CameraComponent::set_viewport_size(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return;
        aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    }

    void CameraComponent::set_aspect_ratio(float aspect) {
        if (aspect <= 0.0f) return;
        aspect_ratio = aspect;
    }

    void CameraComponent::set_fov(float fov_degrees) { fov = fov_degrees; }
    void CameraComponent::set_near_clip(float near_c) { near_clip = near_c; }
    void CameraComponent::set_far_clip(float far_c) { far_clip = far_c; }

    void CameraComponent::update_matrices(const TransformComponent& transform) {
        projection_ = glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip);
        view_ = glm::lookAt(transform.position, transform.position + transform.get_forward(), transform.get_up());
        view_projection_ = projection_ * view_;
    }
}
