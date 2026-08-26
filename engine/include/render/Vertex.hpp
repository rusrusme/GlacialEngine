//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace engine {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 tangent{0.0f};
    };
}
