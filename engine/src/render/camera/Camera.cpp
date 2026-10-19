//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/camera/Camera.hpp"

namespace engine {
    Camera::Camera(float fov_degrees, float aspect_ratio, float near_clip, float far_clip)
        : fov_(fov_degrees), aspect_ratio_(aspect_ratio), near_clip_(near_clip), far_clip_(far_clip) {
        update_projection();
        update_camera();
    }

    void Camera::set_viewport_size(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return;
        aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);
        update_projection();
    }

    void Camera::orbit(float delta_yaw, float delta_pitch) {
        yaw_ += delta_yaw;
        pitch_ = std::clamp(pitch_ + delta_pitch, -89.0f, 89.0f);
        update_camera();
    }

    void Camera::pan(const glm::vec2& delta) {
        const glm::vec3 forward = glm::normalize(target_ - position_);
        const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        const glm::vec3 up = glm::normalize(glm::cross(right, forward));

        target_ += (right * delta.x + up * delta.y);
        update_camera();
    }

    void Camera::zoom(float delta) {
        distance_ = std::max(0.5f, distance_ - delta);
        update_camera();
    }

    void Camera::set_target(const glm::vec3& target) {
        target_ = target;
        update_camera();
    }

    void Camera::set_distance(float distance) {
        distance_ = std::max(0.5f, distance);
        update_camera();
    }

    void Camera::update_camera() {
        const float yaw_rad = glm::radians(yaw_);
        const float pitch_rad = glm::radians(pitch_);

        // Sphärische Koordinaten zu kartesischen Koordinaten
        position_.x = target_.x + distance_ * std::cos(pitch_rad) * std::sin(yaw_rad);
        position_.y = target_.y + distance_ * std::sin(pitch_rad);
        position_.z = target_.z + distance_ * std::cos(pitch_rad) * std::cos(yaw_rad);

        view_ = glm::lookAt(position_, target_, glm::vec3(0.0f, 1.0f, 0.0f));
        view_projection_ = projection_ * view_;
    }

    void Camera::update_projection() {
        projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, near_clip_, far_clip_);
        view_projection_ = projection_ * view_;
    }
}
