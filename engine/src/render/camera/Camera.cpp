//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/camera/Camera.hpp"

namespace engine {
    Camera::Camera(float fov, float aspect, float near_p, float far_p)
        : fov_(fov), aspect_ratio_(aspect), near_clip_(near_p), far_clip_(far_p) {}

    void Camera::set_viewport_size(uint32_t width, uint32_t height) {
        viewport_width_ = width;
        viewport_height_ = height;
        if (height > 0) {
            aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);
        }
    }

    void Camera::set_position(const glm::vec3& position) {
        position_ = position;
    }

    void Camera::set_target(const glm::vec3& target) {
        target_ = target;
    }

    glm::mat4 Camera::view_matrix() const {
        return glm::lookAt(position_, target_, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 Camera::projection_matrix() const {
        return glm::perspective(glm::radians(fov_), aspect_ratio_, near_clip_, far_clip_);
    }

    glm::vec3 Camera::position() const {
        return position_;
    }

    GPUCameraData Camera::gpu_data() const {
        GPUCameraData data{};
        data.view = view_matrix();
        data.projection = projection_matrix();
        data.view_projection = data.projection * data.view;
        data.position = glm::vec4(position_, 1.0f);

        calculate_frustum_planes(data.view_projection, data.frustum_planes);

        data.viewport_size = glm::vec2(viewport_width_, viewport_height_);
        data.near_plane = near_clip_;
        data.far_plane = far_clip_;
        return data;
    }

    void Camera::calculate_frustum_planes(const glm::mat4& vp, glm::vec4 planes[6]) {
        // Gribb-Hartmann method for extracting frustum planes from View-Projection matrix
        // Left Plane
        planes[0] = glm::vec4(
            vp[0][3] + vp[0][0],
            vp[1][3] + vp[1][0],
            vp[2][3] + vp[2][0],
            vp[3][3] + vp[3][0]
        );
        // Right Plane
        planes[1] = glm::vec4(
            vp[0][3] - vp[0][0],
            vp[1][3] - vp[1][0],
            vp[2][3] - vp[2][0],
            vp[3][3] - vp[3][0]
        );
        // Bottom Plane
        planes[2] = glm::vec4(
            vp[0][3] + vp[0][1],
            vp[1][3] + vp[1][1],
            vp[2][3] + vp[2][1],
            vp[3][3] + vp[3][1]
        );
        // Top Plane
        planes[3] = glm::vec4(
            vp[0][3] - vp[0][1],
            vp[1][3] - vp[1][1],
            vp[2][3] - vp[2][1],
            vp[3][3] - vp[3][1]
        );
        // Near Plane
        planes[4] = glm::vec4(
            vp[0][3] + vp[0][2],
            vp[1][3] + vp[1][2],
            vp[2][3] + vp[2][2],
            vp[3][3] + vp[3][2]
        );
        // Far Plane
        planes[5] = glm::vec4(
            vp[0][3] - vp[0][2],
            vp[1][3] - vp[1][2],
            vp[2][3] - vp[2][2],
            vp[3][3] - vp[3][2]
        );

        // Normalize vector normals (Ax + By + Cz + D = 0)
        for (int i = 0; i < 6; ++i) {
            const float length = glm::length(glm::vec3(planes[i]));
            if (length > 0.0001f) {
                planes[i] /= length;
            }
        }
    }
}
