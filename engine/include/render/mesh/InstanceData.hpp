//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace engine {
    struct InstanceData {
        glm::mat4 transform{1.0f};
        glm::vec4 color{1.0f};
    };
}
