//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

#include "render/gpu/GPUData.hpp"

namespace engine {
    class Camera {
    public:
        Camera(float fov = 45.0f, float aspect = 16.0f / 9.0f, float near_p = 0.1f, float far_p = 1000.0f);

        void set_viewport_size(uint32_t width, uint32_t height);
        void set_position(const glm::vec3& position);
        void set_target(const glm::vec3& target);

        [[nodiscard]] glm::mat4 get_view_matrix() const;
        [[nodiscard]] glm::mat4 get_projection_matrix() const;
        [[nodiscard]] glm::vec<3, float> get_position() const;

        [[nodiscard]] GPUCameraData get_gpu_data() const;

    private:
        static void calculate_frustum_planes(const glm::mat4& vp, glm::vec4 planes[6]);

        float fov_{45.0f};
        float aspect_ratio_{1.77f};
        float near_clip_{0.1f};
        float far_clip_{1000.0f};

        uint32_t viewport_width_{1920};
        uint32_t viewport_height_{1080};

        glm::vec3 position_{0.0f, 10.0f, 10.0f};
        glm::vec3 target_{0.0f, 0.0f, 0.0f};
    };
}
