//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/legacy/mesh/Mesh.hpp"

namespace engine {
    class PrimitiveMesh {
    public:
        static Mesh create_plane(float size = 1.0f);

        static Mesh create_circle(float radius = 0.5f, uint32_t segments = 32);

        static Mesh create_cube();

        static Mesh create_uv_sphere(float radius = 0.5f, uint32_t rings = 16, uint32_t sectors = 32);

        static Mesh create_cylinder(float radius = 0.5f, float height = 1.0f, uint32_t segments = 32);

        static Mesh create_pyramid(float base_size = 1.0f, float height = 1.0f);
    };
}
