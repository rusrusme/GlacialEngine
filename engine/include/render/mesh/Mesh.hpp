//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>

#include "physics/AABB.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    struct Vertex {
        glm::vec3 position{0.0f};
        glm::vec3 normal{0.0f};
        glm::vec2 uv{0.0f};
        glm::vec4 tangent{0.0f};
    };

    struct Submesh {
        uint32_t mesh_id{0};             ///< Index in GPUSubmeshData SSBO
        uint32_t first_index{0};         ///< Offset in global Index Buffer
        int32_t base_vertex{0};          ///< Offset in global Vertex Buffer
        uint32_t index_count{0};         ///< Index count for indirect draw command
        uint32_t default_material_id{0}; ///< Reference into MaterialSystem SSBO

        AABB local_bounds;
    };

    class Mesh {
    public:
        Mesh() = default;

        [[nodiscard]] const Vector<Submesh>& submeshes() const { return submeshes_; }
        [[nodiscard]] const AABB& bounds() const { return bounds_; }
        [[nodiscard]] const std::string& name() const { return name_; }

    private:
        friend class MeshLoader;

        std::string name_;
        Vector<Submesh> submeshes_;
        AABB bounds_;
    };
}
