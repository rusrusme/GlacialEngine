//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <algorithm>
#include <glm/glm.hpp>

namespace engine {
    class Camera {
    public:
        Camera(float fov_degrees = 45.0f, float aspect_ratio = 16.0f / 9.0f, float near_clip = 0.1f, float far_clip = 1000.0f);

        void set_viewport_size(uint32_t width, uint32_t height);

        void orbit(float delta_yaw, float delta_pitch);
        void pan(const glm::vec2& delta);
        void zoom(float delta);

        [[nodiscard]] const glm::mat4& view() const noexcept { return view_; }
        [[nodiscard]] const glm::mat4& projection() const noexcept { return projection_; }
        [[nodiscard]] const glm::mat4& view_projection() const noexcept { return view_projection_; }
        [[nodiscard]] const glm::vec3& position() const noexcept { return position_; }
        [[nodiscard]] const glm::vec3& target() const noexcept { return target_; }
        [[nodiscard]] float distance() const noexcept { return distance_; }

        void set_target(const glm::vec3& target);

        void set_distance(float distance);

    private:
        void update_camera();
        void update_projection();

        float fov_{45.0f};
        float aspect_ratio_{1.777f};
        float near_clip_{0.1f};
        float far_clip_{1000.0f};

        glm::vec3 target_{0.0f, 0.0f, 0.0f};
        glm::vec3 position_{0.0f, 2.0f, 5.0f};

        float distance_{5.0f};
        float yaw_{0.0f};
        float pitch_{20.0f};

        glm::mat4 view_{1.0f};
        glm::mat4 projection_{1.0f};
        glm::mat4 view_projection_{1.0f};
    };
}
