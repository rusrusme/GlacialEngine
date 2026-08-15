//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include <cstdint>

namespace engine {
    // Todo: Move mesh entity logic to component (?)
    struct Mesh {
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ebo = 0;
        std::uint32_t index_count = 0;
    };
}
