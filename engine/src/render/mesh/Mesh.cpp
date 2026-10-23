//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <utility>

#include "render/mesh/Mesh.hpp"

namespace engine {
    Mesh::Mesh(
        const RHIDevice& device,
        std::span<const Vertex> vertices,
        std::span<const uint32_t> indices,
        Vector<Submesh> submeshes,
        Vector<Material> materials)
        : vertex_count_(vertices.size()),
          index_count_(indices.size()),
          submeshes_(std::move(submeshes)),
          materials_(std::move(materials)) {
        //////////////////////////////////////////////////////
        vertex_buffer_ = device.create_vertex_buffer(vertices.size_bytes(), vertices.data());
        index_buffer_ = device.create_index_buffer(indices.size_bytes(), indices.data());

        if (!vertices.empty()) {
            bounds_.min = vertices[0].position;
            bounds_.max = vertices[0].position;
            for (const auto& v : vertices) {
                bounds_.min = glm::min(bounds_.min, v.position);
                bounds_.max = glm::max(bounds_.max, v.position);
            }
        }

        if (submeshes_.empty()) {
            submeshes_.push_back(Submesh{
                .index_count    = index_count_,
                .first_index    = 0,
                .vertex_offset  = 0,
                .material_index = 0,
                .bounds         = bounds_
            });
        }
    }

    Shared<Mesh> Mesh::create_cube(const RHIDevice& device) {
        const Vertex vertices[] = {
            // Front (+Z) -> +U is +X
            {.position = {-0.5f, -0.5f, 0.5f}, .normal = {0.0f, 0.0f, 1.0f}, .uv = {0.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, -0.5f, 0.5f}, .normal = {0.0f, 0.0f, 1.0f}, .uv = {1.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, 0.5f, 0.5f}, .normal = {0.0f, 0.0f, 1.0f}, .uv = {1.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, 0.5f, 0.5f}, .normal = {0.0f, 0.0f, 1.0f}, .uv = {0.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            // Back (-Z) -> +U is -X
            {.position = {0.5f, -0.5f, -0.5f}, .normal = {0.0f, 0.0f, -1.0f}, .uv = {0.0f, 0.0f}, .tangent = {-1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, -0.5f, -0.5f}, .normal = {0.0f, 0.0f, -1.0f}, .uv = {1.0f, 0.0f}, .tangent = {-1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, 0.5f, -0.5f}, .normal = {0.0f, 0.0f, -1.0f}, .uv = {1.0f, 1.0f}, .tangent = {-1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, 0.5f, -0.5f}, .normal = {0.0f, 0.0f, -1.0f}, .uv = {0.0f, 1.0f}, .tangent = {-1.0f, 0.0f, 0.0f, 1.0f}},
            // Top (+Y) -> +U is +X
            {.position = {-0.5f, 0.5f, 0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, 0.5f, 0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, 0.5f, -0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, 0.5f, -0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            // Bottom (-Y) -> +U is +X
            {.position = {-0.5f, -0.5f, -0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {0.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, -0.5f, -0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {1.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, -0.5f, 0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {1.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, -0.5f, 0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {0.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            // Right (+X) -> +U is -Z
            {.position = {0.5f, -0.5f, 0.5f}, .normal = {1.0f, 0.0f, 0.0f}, .uv = {0.0f, 0.0f}, .tangent = {0.0f, 0.0f, -1.0f, 1.0f}},
            {.position = {0.5f, -0.5f, -0.5f}, .normal = {1.0f, 0.0f, 0.0f}, .uv = {1.0f, 0.0f}, .tangent = {0.0f, 0.0f, -1.0f, 1.0f}},
            {.position = {0.5f, 0.5f, -0.5f}, .normal = {1.0f, 0.0f, 0.0f}, .uv = {1.0f, 1.0f}, .tangent = {0.0f, 0.0f, -1.0f, 1.0f}},
            {.position = {0.5f, 0.5f, 0.5f}, .normal = {1.0f, 0.0f, 0.0f}, .uv = {0.0f, 1.0f}, .tangent = {0.0f, 0.0f, -1.0f, 1.0f}},
            // Left (-X) -> +U is +Z
            {.position = {-0.5f, -0.5f, -0.5f}, .normal = {-1.0f, 0.0f, 0.0f}, .uv = {0.0f, 0.0f}, .tangent = {0.0f, 0.0f, 1.0f, 1.0f}},
            {.position = {-0.5f, -0.5f, 0.5f}, .normal = {-1.0f, 0.0f, 0.0f}, .uv = {1.0f, 0.0f}, .tangent = {0.0f, 0.0f, 1.0f, 1.0f}},
            {.position = {-0.5f, 0.5f, 0.5f}, .normal = {-1.0f, 0.0f, 0.0f}, .uv = {1.0f, 1.0f}, .tangent = {0.0f, 0.0f, 1.0f, 1.0f}},
            {.position = {-0.5f, 0.5f, -0.5f}, .normal = {-1.0f, 0.0f, 0.0f}, .uv = {0.0f, 1.0f}, .tangent = {0.0f, 0.0f, 1.0f, 1.0f}},
        };

        const uint32_t indices[] = {
            0, 1, 2, 2, 3, 0,       // Front
            4, 5, 6, 6, 7, 4,       // Back
            8, 9, 10, 10, 11, 8,    // Top
            12, 13, 14, 14, 15, 12, // Bottom
            16, 17, 18, 18, 19, 16, // Right
            20, 21, 22, 22, 23, 20  // Left
        };

        return std::make_shared<Mesh>(device, vertices, indices);
    }
}
