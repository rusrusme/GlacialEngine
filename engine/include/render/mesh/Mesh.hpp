//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/Vertex.hpp"
#include "render/VertexArray.hpp"

namespace engine {
    class Mesh {
    public:
        Mesh() = default;
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        std::shared_ptr<VertexArray> vertex_array;

        void setup(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        void draw() const;

        [[nodiscard]] bool is_valid() const;

    private:
        uint32_t vao_ = 0;
        uint32_t vbo_ = 0;
        uint32_t ebo_ = 0;
        uint32_t index_count_ = 0;
    };
}
