//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

#include "render/rhi/Texture.hpp"

namespace engine {
    class Mesh;

    struct RenderObject {
        const Mesh* mesh{nullptr};
        glm::mat4 transform{1.0f};
        glm::vec4 color{1.0f};

        const Texture2D* albedo_texture{nullptr};
        const Texture2D* normal_texture{nullptr};
        const Texture2D* metallic_roughness_texture{nullptr};

        float metallic{0.0f};
        float roughness{1.0f};
        float ao{1.0f};
        float normal_scale{1.0f};
    };
}
