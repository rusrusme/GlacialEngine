//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>

#include "physics/AABB.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/BufferLayout.hpp"
#include "render/rhi/RHIDevice.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    struct Material {
        std::string name;

        Shared<Texture2D> albedo_map;
        Shared<Texture2D> normal_map;
        Shared<Texture2D> metallic_roughness_map;

        glm::vec4 base_color_factor{1.0f};
        float metallic_factor{1.0f};
        float roughness_factor{1.0f};

        bool double_sided{false};
    };

    struct Vertex {
        glm::vec3 position{0.0f};
        glm::vec3 normal{0.0f};
        glm::vec2 uv{0.0f};
        glm::vec4 tangent{0.0f};

        static BufferLayout layout() {
            return {
                {ShaderDataType::Float3, "a_Position"},
                {ShaderDataType::Float3, "a_Normal"},
                {ShaderDataType::Float2, "a_UV"},
                {ShaderDataType::Float4, "a_Color"}
            };
        }
    };

    struct Submesh {
        uint32_t index_count{0};    ///< Index count for indirect draw command
        uint32_t first_index{0};    ///< Offset in global Index Buffer
        int32_t vertex_offset{0};   ///< Offset in global Vertex Buffer
        uint32_t material_index{0}; ///< Reference into MaterialSystem SSBO

        AABB bounds;
    };

    class Mesh {
    public:
        Mesh(const RHIDevice& device,
             std::span<const Vertex> vertices,
             std::span<const uint32_t> indices,
             Vector<Submesh> submeshes = {},
             Vector<Material> materials = {});

        /** @brief For debug purposes */
        [[nodiscard]] static Shared<Mesh> create_cube(const RHIDevice& device);

        [[nodiscard]] const VertexBuffer& vertex_buffer() const noexcept { return *vertex_buffer_; }
        [[nodiscard]] const IndexBuffer& index_buffer() const noexcept { return *index_buffer_; }
        [[nodiscard]] uint32_t vertex_count() const noexcept { return vertex_count_; }
        [[nodiscard]] uint32_t index_count() const noexcept { return index_count_; }

        [[nodiscard]] const Vector<Submesh>& submeshes() const noexcept { return submeshes_; }
        [[nodiscard]] const AABB& bounds() const noexcept { return bounds_; }
        [[nodiscard]] const Vector<Material>& materials() const noexcept { return materials_; }

    private:
        Unique<VertexBuffer> vertex_buffer_{nullptr};
        Unique<IndexBuffer> index_buffer_{nullptr};
        uint32_t vertex_count_{0};
        uint32_t index_count_{0};
        Vector<Submesh> submeshes_{};
        Vector<Material> materials_{};
        AABB bounds_{};
    };
}
