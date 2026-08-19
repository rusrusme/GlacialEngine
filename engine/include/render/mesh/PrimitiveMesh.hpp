//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/gtc/constants.hpp>
#include <render/Renderer.hpp>
#include <render/mesh/Mesh.hpp>

namespace engine {
    class PrimitiveMesh {
    public:
        static Mesh create_plane(const float size = 1.0f) {
            float half = size * 0.5f;
            std::vector<Vertex> vertices = {
                // Pos                    // Normal            // UV
                {.position = {-half, 0.0f, half}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 0.0f}},
                {.position = {half, 0.0f, half}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 0.0f}},
                {.position = {half, 0.0f, -half}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 1.0f}},
                {.position = {-half, 0.0f, -half}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 1.0f}}
            };
            std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
            return Renderer::upload_mesh(vertices, indices);
        }

        static Mesh create_circle(const float radius = 0.5f, const uint32_t segments = 32) {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            vertices.push_back({.position = {0.0f, 0.0f, 0.0f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.5f, 0.5f}});

            const float angle_step = glm::two_pi<float>() / static_cast<float>(segments);

            for (uint32_t i = 0; i <= segments; ++i) {
                float angle = static_cast<float>(i) * angle_step;
                float x = std::cos(angle);
                float z = std::sin(angle);

                glm::vec3 pos(x * radius, 0.0f, z * radius);
                glm::vec3 normal(0.0f, 1.0f, 0.0f);
                glm::vec2 uv(x * 0.5f + 0.5f, z * 0.5f + 0.5f);

                vertices.push_back({pos, normal, uv});
            }
            for (uint32_t i = 1; i <= segments; ++i) {
                indices.push_back(0);
                indices.push_back(i);
                indices.push_back(i + 1);
            }
            return Renderer::upload_mesh(vertices, indices);
        }

        static Mesh create_cube() {
            std::vector<Vertex> vertices = {
                // Front Face (Normal: 0, 0, 1)
                {.position = {-0.5f, -0.5f, 0.5f}, .normal = {0.0f, 0.0f, 1.0f}, .uv = {0.0f, 0.0f}},
                {.position = {0.5f, -0.5f, 0.5f}, .normal = {0.0f, 0.0f, 1.0f}, .uv = {1.0f, 0.0f}},
                {.position = {0.5f, 0.5f, 0.5f}, .normal = {0.0f, 0.0f, 1.0f}, .uv = {1.0f, 1.0f}},
                {.position = {-0.5f, 0.5f, 0.5f}, .normal = {0.0f, 0.0f, 1.0f}, .uv = {0.0f, 1.0f}},

                // Back Face (Normal: 0, 0, -1)
                {.position = {0.5f, -0.5f, -0.5f}, .normal = {0.0f, 0.0f, -1.0f}, .uv = {0.0f, 0.0f}},
                {.position = {-0.5f, -0.5f, -0.5f}, .normal = {0.0f, 0.0f, -1.0f}, .uv = {1.0f, 0.0f}},
                {.position = {-0.5f, 0.5f, -0.5f}, .normal = {0.0f, 0.0f, -1.0f}, .uv = {1.0f, 1.0f}},
                {.position = {0.5f, 0.5f, -0.5f}, .normal = {0.0f, 0.0f, -1.0f}, .uv = {0.0f, 1.0f}},

                // Top Face (Normal: 0, 1, 0)
                {.position = {-0.5f, 0.5f, 0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 0.0f}},
                {.position = {0.5f, 0.5f, 0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 0.0f}},
                {.position = {0.5f, 0.5f, -0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 1.0f}},
                {.position = {-0.5f, 0.5f, -0.5f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.0f, 1.0f}},

                // Bottom Face (Normal: 0, -1, 0)
                {.position = {-0.5f, -0.5f, -0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {0.0f, 0.0f}},
                {.position = {0.5f, -0.5f, -0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {1.0f, 0.0f}},
                {.position = {0.5f, -0.5f, 0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {1.0f, 1.0f}},
                {.position = {-0.5f, -0.5f, 0.5f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {0.0f, 1.0f}},

                // Right Face (Normal: 1, 0, 0)
                {.position = {0.5f, -0.5f, 0.5f}, .normal = {1.0f, 0.0f, 0.0f}, .uv = {0.0f, 0.0f}},
                {.position = {0.5f, -0.5f, -0.5f}, .normal = {1.0f, 0.0f, 0.0f}, .uv = {1.0f, 0.0f}},
                {.position = {0.5f, 0.5f, -0.5f}, .normal = {1.0f, 0.0f, 0.0f}, .uv = {1.0f, 1.0f}},
                {.position = {0.5f, 0.5f, 0.5f}, .normal = {1.0f, 0.0f, 0.0f}, .uv = {0.0f, 1.0f}},

                // Left Face (Normal: -1, 0, 0)
                {.position = {-0.5f, -0.5f, -0.5f}, .normal = {-1.0f, 0.0f, 0.0f}, .uv = {0.0f, 0.0f}},
                {.position = {-0.5f, -0.5f, 0.5f}, .normal = {-1.0f, 0.0f, 0.0f}, .uv = {1.0f, 0.0f}},
                {.position = {-0.5f, 0.5f, 0.5f}, .normal = {-1.0f, 0.0f, 0.0f}, .uv = {1.0f, 1.0f}},
                {.position = {-0.5f, 0.5f, -0.5f}, .normal = {-1.0f, 0.0f, 0.0f}, .uv = {0.0f, 1.0f}}
            };

            std::vector<uint32_t> indices = {
                0, 1, 2, 2, 3, 0,       // Front
                4, 5, 6, 6, 7, 4,       // Back
                8, 9, 10, 10, 11, 8,    // Top
                12, 13, 14, 14, 15, 12, // Bottom
                16, 17, 18, 18, 19, 16, // Right
                20, 21, 22, 22, 23, 20  // Left
            };

            return Renderer::upload_mesh(vertices, indices);
        }

        static Mesh create_uv_sphere(const float radius = 0.5f, const uint32_t rings = 16, const uint32_t sectors = 32) {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            float R = 1.0f / static_cast<float>(rings - 1);
            float S = 1.0f / static_cast<float>(sectors - 1);

            for (uint32_t r = 0; r < rings; ++r) {
                for (uint32_t s = 0; s < sectors; ++s) {
                    float y = std::sin(-glm::half_pi<float>() + glm::pi<float>() * r * R);
                    float x = std::cos(2 * glm::pi<float>() * s * S) * std::sin(glm::pi<float>() * r * R);
                    float z = std::sin(2 * glm::pi<float>() * s * S) * std::sin(glm::pi<float>() * r * R);

                    glm::vec3 pos(x, y, z);
                    glm::vec3 normal = glm::normalize(pos);
                    glm::vec2 uv(s * S, r * R);

                    vertices.push_back({.position = pos * radius, .normal = normal, .uv = uv});
                }
            }
            for (uint32_t r = 0; r < rings - 1; ++r) {
                for (uint32_t s = 0; s < sectors - 1; ++s) {
                    uint32_t i0 = r * sectors + s;
                    uint32_t i1 = r * sectors + (s + 1);
                    uint32_t i2 = (r + 1) * sectors + (s + 1);
                    uint32_t i3 = (r + 1) * sectors + s;

                    indices.push_back(i0);
                    indices.push_back(i1);
                    indices.push_back(i2);
                    indices.push_back(i0);
                    indices.push_back(i2);
                    indices.push_back(i3);
                }
            }
            return Renderer::upload_mesh(vertices, indices);
        }

        static Mesh create_cylinder(const float radius = 0.5f, const float height = 1.0f, const uint32_t segments = 32) {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            const float half_h = height * 0.5f;
            const float angle_step = glm::two_pi<float>() / static_cast<float>(segments);

            for (uint32_t i = 0; i <= segments; ++i) {
                float angle = static_cast<float>(i) * angle_step;
                float x = std::cos(angle);
                float z = std::sin(angle);

                glm::vec3 normal(x, 0.0f, z); // Normalen zeigen nach außen
                float u = static_cast<float>(i) / static_cast<float>(segments);

                vertices.push_back({.position = {x * radius, half_h, z * radius}, .normal = normal, .uv = {u, 1.0f}});  // Top
                vertices.push_back({.position = {x * radius, -half_h, z * radius}, .normal = normal, .uv = {u, 0.0f}}); // Bottom
            }
            for (uint32_t i = 0; i < segments; ++i) {
                uint32_t top_left = i * 2;
                uint32_t bottom_left = top_left + 1;
                uint32_t top_right = top_left + 2;
                uint32_t bottom_right = top_left + 3;

                indices.push_back(top_left);
                indices.push_back(bottom_left);
                indices.push_back(top_right);

                indices.push_back(top_right);
                indices.push_back(bottom_left);
                indices.push_back(bottom_right);
            }

            auto top_center_idx = static_cast<uint32_t>(vertices.size());
            vertices.push_back({.position = {0.0f, half_h, 0.0f}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {0.5f, 0.5f}});

            for (uint32_t i = 0; i <= segments; ++i) {
                float angle = static_cast<float>(i) * angle_step;
                float x = std::cos(angle);
                float z = std::sin(angle);

                vertices.push_back({.position = {x * radius, half_h, z * radius}, .normal = {0.0f, 1.0f, 0.0f}, .uv = {x * 0.5f + 0.5f, z * 0.5f + 0.5f}});
            }
            for (uint32_t i = 1; i <= segments; ++i) {
                indices.push_back(top_center_idx);
                indices.push_back(top_center_idx + i);
                indices.push_back(top_center_idx + i + 1);
            }

            // ----------------------------------------------------
            // C. BODEN (Bottom Cap)
            // ----------------------------------------------------
            auto bottom_center_idx = static_cast<uint32_t>(vertices.size());
            vertices.push_back({.position = {0.0f, -half_h, 0.0f}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {0.5f, 0.5f}});

            for (uint32_t i = 0; i <= segments; ++i) {
                float angle = static_cast<float>(i) * angle_step;
                float x = std::cos(angle);
                float z = std::sin(angle);

                vertices.push_back({.position = {x * radius, -half_h, z * radius}, .normal = {0.0f, -1.0f, 0.0f}, .uv = {x * 0.5f + 0.5f, z * 0.5f + 0.5f}});
            }
            for (uint32_t i = 1; i <= segments; ++i) {
                // Winding Order gedreht für Unterseite
                indices.push_back(bottom_center_idx);
                indices.push_back(bottom_center_idx + i + 1);
                indices.push_back(bottom_center_idx + i);
            }
            return Renderer::upload_mesh(vertices, indices);
        }
    };
}
