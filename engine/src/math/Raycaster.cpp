//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "math/Raycaster.hpp"

#include "math/Ray.hpp"
#include "render/legacy/Camera.hpp"

namespace engine {
    Ray Raycaster::cast_ray(const glm::vec2 mouse_pos, const int viewport_width, const int viewport_height, const Camera& camera) {
        const float ndc_x = 2.0f * mouse_pos.x / static_cast<float>(viewport_width) - 1.0f;
        const float ndc_y = 1.0f - 2.0f * mouse_pos.y / static_cast<float>(viewport_height);

        const glm::vec4 ray_clip(ndc_x, ndc_y, -1.0f, 1.0f);

        glm::vec4 ray_eye = glm::inverse(camera.get_projection_matrix()) * ray_clip;
        ray_eye.z = -1.0f;
        ray_eye.w = 0.0f;

        const glm::vec4 ray_world4 = glm::inverse(camera.get_view_matrix()) * ray_eye;
        const glm::vec3 ray_world = glm::normalize(glm::vec3(ray_world4));

        return Ray{.origin = camera.get_position(), .direction = ray_world};
    }
}
