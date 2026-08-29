//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "physics/AABB.hpp"

namespace engine {
    class Texture2D;
    struct Vertex;
    class VertexArray;

    class Mesh {
    public:
        Mesh() = default;
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        std::shared_ptr<VertexArray> vertex_array = nullptr;
        std::shared_ptr<Texture2D> texture = nullptr;
        std::shared_ptr<Texture2D> normal_map = nullptr;
        std::shared_ptr<Texture2D> metallic_roughness_map = nullptr;
        AABB local_bounds;

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
