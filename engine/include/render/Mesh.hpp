//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include <cstdint>

namespace engine {
    struct Mesh {
        std::uint32_t vao = 0;
        std::uint32_t vbo = 0;
        std::uint32_t ebo = 0;
        std::uint32_t index_count = 0;
    };
}
