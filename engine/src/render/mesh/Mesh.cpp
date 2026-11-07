//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <cmath>
#include <utility>
#include <vector>
#include <glm/gtc/constants.hpp>

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

    Shared<Mesh> Mesh::create_plane(const RHIDevice& device) {
        // Horizontal 1x1 plane on XZ facing +Y (Up)
        const Vertex vertices[] = {
            {.position = {-0.5f, 0.0f, 0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, 0.0f, 0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, 0.0f, -0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, 0.0f, -0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
        };

        const uint32_t indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        return std::make_shared<Mesh>(device, vertices, indices);
    }

    Shared<Mesh> Mesh::create_circle(const RHIDevice& device, uint32_t segments) {
        if (segments < 3) {
            segments = 3;
        }

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        vertices.reserve(segments + 1);
        indices.reserve(segments * 3);

        constexpr float radius = 0.5f;

        // Center vertex
        vertices.push_back(Vertex{
            .position = {0.0f, 0.0f, 0.0f},
            .normal   = {0.0f, 1.0f, 0.0f},
            .uv       = {0.5f, 0.5f},
            .tangent  = {1.0f, 0.0f, 0.0f, 1.0f}
        });

        // Rim vertices (XZ plane facing +Y)
        for (uint32_t i = 0; i < segments; ++i) {
            const float angle = (static_cast<float>(i) / static_cast<float>(segments)) * glm::two_pi<float>();
            const float cos_a = std::cos(angle);
            const float sin_a = std::sin(angle);

            const float x = radius * cos_a;
            const float z = -radius * sin_a;

            vertices.push_back(Vertex{
                .position = {x, 0.0f, z},
                .normal   = {0.0f, 1.0f, 0.0f},
                .uv       = {0.5f + (cos_a * 0.5f), 0.5f + (sin_a * 0.5f)},
                .tangent  = {1.0f, 0.0f, 0.0f, 1.0f}
            });
        }

        for (uint32_t i = 0; i < segments; ++i) {
            const uint32_t current = i + 1;
            const uint32_t next = ((i + 1) % segments) + 1;

            indices.push_back(0);
            indices.push_back(current);
            indices.push_back(next);
        }

        return std::make_shared<Mesh>(device, vertices, indices);
    }

    Shared<Mesh> Mesh::create_sphere(const RHIDevice& device, uint32_t latitude_segments, uint32_t longitude_segments) {
        if (latitude_segments < 2) latitude_segments = 2;
        if (longitude_segments < 3) longitude_segments = 3;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        vertices.reserve((latitude_segments + 1) * (longitude_segments + 1));
        indices.reserve(latitude_segments * longitude_segments * 6);

        constexpr float radius = 0.5f;

        for (uint32_t lat = 0; lat <= latitude_segments; ++lat) {
            const float v = static_cast<float>(lat) / static_cast<float>(latitude_segments);
            const float theta = v * glm::pi<float>();
            const float sin_theta = std::sin(theta);
            const float cos_theta = std::cos(theta);

            for (uint32_t lon = 0; lon <= longitude_segments; ++lon) {
                const float u = static_cast<float>(lon) / static_cast<float>(longitude_segments);
                const float phi = u * glm::two_pi<float>();
                const float sin_phi = std::sin(phi);
                const float cos_phi = std::cos(phi);

                const glm::vec3 normal{sin_theta * sin_phi, cos_theta, sin_theta * cos_phi};
                const glm::vec3 position = normal * radius;
                const glm::vec4 tangent{cos_phi, 0.0f, -sin_phi, 1.0f};

                vertices.push_back(Vertex{
                    .position = position,
                    .normal   = normal,
                    .uv       = {u, 1.0f - v},
                    .tangent  = tangent
                });
            }
        }

        const uint32_t stride = longitude_segments + 1;
        for (uint32_t lat = 0; lat < latitude_segments; ++lat) {
            for (uint32_t lon = 0; lon < longitude_segments; ++lon) {
                const uint32_t p00 = lat * stride + lon;
                const uint32_t p01 = p00 + 1;
                const uint32_t p10 = (lat + 1) * stride + lon;
                const uint32_t p11 = p10 + 1;

                if (lat != 0) {
                    indices.push_back(p01);
                    indices.push_back(p00);
                    indices.push_back(p10);
                }
                if (lat != latitude_segments - 1) {
                    indices.push_back(p10);
                    indices.push_back(p11);
                    indices.push_back(p01);
                }
            }
        }

        return std::make_shared<Mesh>(device, vertices, indices);
    }

    Shared<Mesh> Mesh::create_cylinder(const RHIDevice& device, uint32_t segments) {
        if (segments < 3) {
            segments = 3;
        }

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        constexpr float radius = 0.5f;
        constexpr float half_height = 0.5f;

        // Barrel Body
        const auto side_start_index = static_cast<uint32_t>(vertices.size());
        for (uint32_t i = 0; i <= segments; ++i) {
            const float u = static_cast<float>(i) / static_cast<float>(segments);
            const float phi = u * glm::two_pi<float>();
            const float cos_phi = std::cos(phi);
            const float sin_phi = std::sin(phi);

            const float x = radius * sin_phi;
            const float z = radius * cos_phi;
            const glm::vec3 normal{sin_phi, 0.0f, cos_phi};
            const glm::vec4 tangent{cos_phi, 0.0f, -sin_phi, 1.0f};

            vertices.push_back(Vertex{
                .position = {x, half_height, z},
                .normal   = normal,
                .uv       = {u, 1.0f},
                .tangent  = tangent
            });

            vertices.push_back(Vertex{
                .position = {x, -half_height, z},
                .normal   = normal,
                .uv       = {u, 0.0f},
                .tangent  = tangent
            });
        }

        for (uint32_t i = 0; i < segments; ++i) {
            const uint32_t top_left = side_start_index + (i * 2);
            const uint32_t bottom_left = top_left + 1;
            const uint32_t top_right = side_start_index + ((i + 1) * 2);
            const uint32_t bottom_right = top_right + 1;

            indices.push_back(bottom_left);
            indices.push_back(bottom_right);
            indices.push_back(top_right);

            indices.push_back(top_right);
            indices.push_back(top_left);
            indices.push_back(bottom_left);
        }

        // Top Cap (+Y)
        const auto top_center_index = static_cast<uint32_t>(vertices.size());
        vertices.push_back(Vertex{
            .position = {0.0f, half_height, 0.0f},
            .normal   = {0.0f, 1.0f, 0.0f},
            .uv       = {0.5f, 0.5f},
            .tangent  = {1.0f, 0.0f, 0.0f, 1.0f}
        });

        const auto top_ring_start = static_cast<uint32_t>(vertices.size());
        for (uint32_t i = 0; i <= segments; ++i) {
            const float phi = (static_cast<float>(i) / static_cast<float>(segments)) * glm::two_pi<float>();
            const float x = radius * std::sin(phi);
            const float z = radius * std::cos(phi);

            vertices.push_back(Vertex{
                .position = {x, half_height, z},
                .normal   = {0.0f, 1.0f, 0.0f},
                .uv       = {0.5f + (x / (2.0f * radius)), 0.5f + (z / (2.0f * radius))},
                .tangent  = {1.0f, 0.0f, 0.0f, 1.0f}
            });
        }

        for (uint32_t i = 0; i < segments; ++i) {
            indices.push_back(top_center_index);
            indices.push_back(top_ring_start + i + 1);
            indices.push_back(top_ring_start + i);
        }

        // Bottom Cap (-Y)
        const auto bottom_center_index = static_cast<uint32_t>(vertices.size());
        vertices.push_back(Vertex{
            .position = {0.0f, -half_height, 0.0f},
            .normal   = {0.0f, -1.0f, 0.0f},
            .uv       = {0.5f, 0.5f},
            .tangent  = {1.0f, 0.0f, 0.0f, 1.0f}
        });

        const auto bottom_ring_start = static_cast<uint32_t>(vertices.size());
        for (uint32_t i = 0; i <= segments; ++i) {
            const float phi = (static_cast<float>(i) / static_cast<float>(segments)) * glm::two_pi<float>();
            const float x = radius * std::sin(phi);
            const float z = radius * std::cos(phi);

            vertices.push_back(Vertex{
                .position = {x, -half_height, z},
                .normal   = {0.0f, -1.0f, 0.0f},
                .uv       = {0.5f + (x / (2.0f * radius)), 0.5f - (z / (2.0f * radius))},
                .tangent  = {1.0f, 0.0f, 0.0f, 1.0f}
            });
        }

        for (uint32_t i = 0; i < segments; ++i) {
            indices.push_back(bottom_center_index);
            indices.push_back(bottom_ring_start + i);
            indices.push_back(bottom_ring_start + i + 1);
        }

        return std::make_shared<Mesh>(device, vertices, indices);
    }

    Shared<Mesh> Mesh::create_pyramid(const RHIDevice& device) {
        // Face normals (slope has run 0.5 for rise 1.0)
        const glm::vec3 n_front = glm::normalize(glm::vec3(0.0f, 0.5f, 1.0f));
        const glm::vec3 n_back = glm::normalize(glm::vec3(0.0f, 0.5f, -1.0f));
        const glm::vec3 n_right = glm::normalize(glm::vec3(1.0f, 0.5f, 0.0f));
        const glm::vec3 n_left = glm::normalize(glm::vec3(-1.0f, 0.5f, 0.0f));

        const Vertex vertices[] = {
            // Bottom (-Y)
            {.position = {-0.5f, -0.5f, -0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {0.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, -0.5f, -0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {1.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, -0.5f, 0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {1.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, -0.5f, 0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {0.0f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},

            // Front (+Z)
            {.position = {-0.5f, -0.5f, 0.5f}, .normal = n_front, .uv = {0.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.5f, -0.5f, 0.5f}, .normal = n_front, .uv = {1.0f, 0.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.0f, 0.5f, 0.0f}, .normal = n_front, .uv = {0.5f, 1.0f}, .tangent = {1.0f, 0.0f, 0.0f, 1.0f}},

            // Right (+X)
            {.position = {0.5f, -0.5f, 0.5f}, .normal = n_right, .uv = {0.0f, 0.0f}, .tangent = {0.0f, 0.0f, -1.0f, 1.0f}},
            {.position = {0.5f, -0.5f, -0.5f}, .normal = n_right, .uv = {1.0f, 0.0f}, .tangent = {0.0f, 0.0f, -1.0f, 1.0f}},
            {.position = {0.0f, 0.5f, 0.0f}, .normal = n_right, .uv = {0.5f, 1.0f}, .tangent = {0.0f, 0.0f, -1.0f, 1.0f}},

            // Back (-Z)
            {.position = {0.5f, -0.5f, -0.5f}, .normal = n_back, .uv = {0.0f, 0.0f}, .tangent = {-1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {-0.5f, -0.5f, -0.5f}, .normal = n_back, .uv = {1.0f, 0.0f}, .tangent = {-1.0f, 0.0f, 0.0f, 1.0f}},
            {.position = {0.0f, 0.5f, 0.0f}, .normal = n_back, .uv = {0.5f, 1.0f}, .tangent = {-1.0f, 0.0f, 0.0f, 1.0f}},

            // Left (-X)
            {.position = {-0.5f, -0.5f, -0.5f}, .normal = n_left, .uv = {0.0f, 0.0f}, .tangent = {0.0f, 0.0f, 1.0f, 1.0f}},
            {.position = {-0.5f, -0.5f, 0.5f}, .normal = n_left, .uv = {1.0f, 0.0f}, .tangent = {0.0f, 0.0f, 1.0f, 1.0f}},
            {.position = {0.0f, 0.5f, 0.0f}, .normal = n_left, .uv = {0.5f, 1.0f}, .tangent = {0.0f, 0.0f, 1.0f, 1.0f}},
        };

        const uint32_t indices[] = {
            // Bottom
            0, 1, 2, 2, 3, 0,
            // Front
            4, 5, 6,
            // Right
            7, 8, 9,
            // Back
            10, 11, 12,
            // Left
            13, 14, 15
        };

        return std::make_shared<Mesh>(device, vertices, indices);
    }
}
