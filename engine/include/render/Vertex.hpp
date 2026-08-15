//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace engine {
    // Todo: Same with vertex (see Mesh.hpp)
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };
}
