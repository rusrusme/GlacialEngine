//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/mesh/Mesh.hpp"

#include "render/legacy/Buffer.hpp"
#include "render/legacy/BufferLayout.hpp"
#include "render/legacy/Vertex.hpp"
#include "render/legacy/VertexArray.hpp"

namespace engine::legacy {
    using namespace engine;
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        setup(vertices, indices);
    }

    void Mesh::setup(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        if (vertices.empty() || indices.empty()) return;

        local_bounds = {
            glm::vec3(std::numeric_limits<float>::max()),
            glm::vec3(std::numeric_limits<float>::lowest())
        };
        for (const auto& vertex : vertices) {
            local_bounds.min = glm::min(local_bounds.min, vertex.position);
            local_bounds.max = glm::max(local_bounds.max, vertex.position);
        }

        // 1. VertexArray erzeugen
        vertex_array = std::make_shared<VertexArray>();

        // 2. VertexBuffer mit Daten befüllen
        auto vertex_buffer = std::make_shared<GLVertexBuffer>(
            vertices.data(),
            vertices.size() * sizeof(Vertex)
        );

        // 3. Layout definieren (Position, Normal, UV)
        BufferLayout layout = {
            {ShaderDataType::Float3, "a_position"},
            {ShaderDataType::Float3, "a_normal"},
            {ShaderDataType::Float2, "a_uv"},
            {ShaderDataType::Float3, "a_tangent"}
        };

        vertex_array->add_vertex_buffer(vertex_buffer, layout);

        // 4. IndexBuffer zuweisen
        auto index_buffer = std::make_shared<GLIndexBuffer>(
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
