//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/vec2.hpp>
#include "math/Ray.hpp"

namespace engine {
    class Camera;

    class Raycaster {
    public:
        /** @brief Casts a ray from the camera through the mouse position into the 3d world */
        static Ray cast_ray(glm::vec2 mouse_pos, int viewport_width, int viewport_height, const Camera& camera);

        // Todo: intersect with terrain
        // Todo: terrain representation in /world (?)
    };
}
