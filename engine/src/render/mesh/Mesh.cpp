//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/mesh/Mesh.hpp"

#include <glad/glad.h>

namespace engine {
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        setup(vertices, indices);
    }

    void Mesh::setup(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        if (vertices.empty() || indices.empty()) return;

        // 1. VertexArray erzeugen
        vertex_array = std::make_shared<VertexArray>();

        // 2. VertexBuffer mit Daten befüllen
        auto vertex_buffer = std::make_shared<VertexBuffer>(
            vertices.data(),
            vertices.size() * sizeof(Vertex)
        );

        // 3. Layout definieren (Position, Normal, UV)
        BufferLayout layout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float2, "a_UV"}
        };

        vertex_array->add_vertex_buffer(vertex_buffer, layout);

        // 4. IndexBuffer zuweisen
        auto index_buffer = std::make_shared<IndexBuffer>(
            indices.data(),
            static_cast<uint32_t>(indices.size())
        );

        vertex_array->set_index_buffer(index_buffer);
    }

    void Mesh::draw() const {
        if (!is_valid()) return;

        vertex_array->bind();
        vertex_array->unbind();
    }

    bool Mesh::is_valid() const {
        return vertex_array != nullptr && vertex_array->get_index_buffer() != nullptr;
    }
}
