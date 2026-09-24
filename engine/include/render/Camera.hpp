//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

#include "math/Ray.hpp"

namespace engine {
    struct Frustum;

    class Camera {
    public:
        void set_target(const glm::vec3& target);

        void set_distance(float distance);

        void set_yaw_pitch(float yaw_degrees, float pitch_degrees);

        void apply_default_view();

        // ---------------------------
        // Movement
        // ---------------------------
        void orbit(float delta_yaw_degrees, float delta_pitch_degrees);
        void pan(float delta_right, float delta_forward);
        void zoom(float delta_distance);
        // ---------------------------

        [[nodiscard]] glm::mat4 get_view_matrix() const;

        [[nodiscard]] glm::mat4 get_projection_matrix() const;

        [[nodiscard]] glm::vec3 get_position() const;

        [[nodiscard]] Frustum get_frustum() const;

        [[nodiscard]] glm::vec3 get_target() const { return target_; }

        [[nodiscard]] float get_fov() const;
        [[nodiscard]] float get_aspect_ratio() const;
        [[nodiscard]] float get_near_clip() const;
        [[nodiscard]] float get_far_clip() const;

        [[nodiscard]] float get_distance() const;

        void set_aspect_ratio(float aspect);
        void set_fov(float fov);
        void set_near_clip(float near_clip);
        void set_far_clip(float far_clip);

        void set_projection_params(float fov, float near_p, float far_p);

        // ---------------------------
        [[nodiscard]] Ray screen_point_to_ray(const glm::vec2& mouse_pos, const glm::vec2& viewport_size) const;

    private:
        void update_position();

        float fov_y_ = 45.0f;
        float aspect_ratio_ = 1.0f;
        float near_clip_ = 0.1f;
        float far_clip_ = 10000.0f;

        glm::vec3 target_{0.0f, 0.0f, 0.0f};
        glm::vec3 position_{0.0f, 10.0f, 10.0f};

        float distance_ = 20.0f;
        float yaw_degrees_ = 45.0f;
        float pitch_degrees_ = 45.0f;
    };
}
