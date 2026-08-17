//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "VertexArray.hpp"

namespace engine {
    struct Mesh {
        std::shared_ptr<VertexArray> vertex_array;

        [[nodiscard]] bool is_valid() const {
            return vertex_array != nullptr && vertex_array->get_index_buffer() != nullptr;
        }
    };
}
