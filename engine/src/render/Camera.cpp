//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/Camera.hpp"

#include <algorithm>
#include <core/Config.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace engine {
    namespace {
        constexpr float PI = 3.1415926535f;

        constexpr float deg_to_rad(const float degrees) {
            return degrees * PI / 180.0f;
        }
    }

    void Camera::set_target(const glm::vec3& target) {
        target_ = target;
        update_position();
    }

    void Camera::set_distance(const float distance) {
        distance_ = std::max(1.0f, distance);
        update_position();
    }

    void Camera::set_yaw_pitch(const float yaw_degrees, const float pitch_degrees) {
        yaw_degrees_ = yaw_degrees;
        pitch_degrees_ = std::clamp(pitch_degrees, 5.0f, 89.0f);
        update_position();
    }

    void Camera::apply_default_view() {
        set_distance(setting::camera::DEFAULT_DISTANCE);
        set_yaw_pitch(setting::camera::DEFAULT_YAW, setting::camera::DEFAULT_PITCH);
    }

    void Camera::orbit(const float delta_yaw_degrees, const float delta_pitch_degrees) {
        yaw_degrees_ += delta_yaw_degrees;
        pitch_degrees_ = std::clamp(pitch_degrees_ + delta_pitch_degrees, 5.0f, 89.0f);
        update_position();
    }

    void Camera::pan(const float delta_right, const float delta_forward) {
        const float yaw_radians = deg_to_rad(yaw_degrees_);

        glm::vec3 forward{
            std::cos(yaw_radians),
            0.0f,
            std::sin(yaw_radians)
        };
        forward = glm::normalize(forward);

        glm::vec3 right{
            -forward.z,
            0.0f,
            forward.x
        };
        right = glm::normalize(right);

        target_ += right * delta_right;
        target_ += forward * delta_forward;

        update_position();
    }

    void Camera::zoom(const float delta_distance) {
        // distance_ *= 1.0f + delta_distance * 0.05f;
        // distance_ = std::max(1.0f, distance_);
        distance_ = std::max(1.0f, distance_ + delta_distance);
        update_position();
    }

    glm::mat4 Camera::get_view_matrix() const {
        return glm::lookAt(position_, target_, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 Camera::get_projection_matrix() const {
        return glm::perspective(fov_y_, aspect_ratio_, near_clip_, far_clip_);
    }

    glm::vec3 Camera::get_position() const {
        return position_;
    }

    float Camera::get_far_clip() const {
        return far_clip_;
    }

    float Camera::get_distance() const {
        return distance_;
    }

    void Camera::set_aspect_ratio(const float aspect) {
        aspect_ratio_ = aspect;
    }

    void Camera::set_projection_params(const float fov, const float near_p, const float far_p) {
        fov_y_ = glm::radians(fov); // glm::perspective expects radians!
        near_clip_ = near_p;
        far_clip_ = far_p;
    }

    Ray Camera::screen_point_to_ray(const glm::vec2& mouse_pos, const glm::vec2& viewport_size) {
        const float x = (2.0f * mouse_pos.x) / viewport_size.x - 1.0f;
        const float y = 1.0f - (2.0f * mouse_pos.y) / viewport_size.y;

        const glm::vec4 ray_start_ndc(x, y, -1.0f, 1.0f);
        const glm::vec4 ray_end_ndc(x, y, 1.0f, 1.0f);

        const glm::mat4 inv_view_proj = glm::inverse(get_projection_matrix() * get_view_matrix());

        glm::vec4 ray_start_world = inv_view_proj * ray_start_ndc;
        ray_start_world /= ray_start_world.w;

        glm::vec4 ray_end_world = inv_view_proj * ray_end_ndc;
        ray_end_world /= ray_end_world.w;

        const glm::vec3 ray_dir = glm::normalize(glm::vec3(ray_end_world - ray_start_world));

        return Ray{.origin = glm::vec3(ray_start_world), .direction = ray_dir};
    }

    void Camera::update_position() {
        const float yaw_radians = deg_to_rad(yaw_degrees_);
        const float pitch_radians = deg_to_rad(pitch_degrees_);

        position_.x = target_.x + distance_ * std::cos(pitch_radians) * std::cos(yaw_radians);
        position_.y = target_.y + distance_ * std::sin(pitch_radians);
        position_.z = target_.z + distance_ * std::cos(pitch_radians) * std::sin(yaw_radians);
    }
}
