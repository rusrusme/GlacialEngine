//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

namespace engine {
    /**
     * @brief Native indirect draw command structure for GPU-driven rendering.
     * @note Matches exact memory layout required by glMultiDrawElementsIndirect and vkCmdDrawIndexedIndirect.
     */
    struct alignas(16) GPUDrawCommand {
        uint32_t count{0};          ///< Number of indices to draw
        uint32_t instance_count{0}; ///< Dynamically toggled by Compute Culling (0 = culled, 1 = visible)
        uint32_t first_index{0};    ///< Byte offset in the Index Buffer / sizeof(IndexType)
        int32_t base_vertex{0};     ///< Offset added to vertex indices
        uint32_t base_instance{0};  ///< Instance ID offset (used for gl_BaseInstance / gl_DrawID lookup)
    };

    /**
     * @brief Per-instance data stored in SSBO (std430 layout).
     * @note Aligned to 128 bytes to maintain strict 16-byte field alignments.
     */
    struct alignas(16) GPUInstanceData {
        glm::mat4 model_matrix{1.0f};           ///< World transformation matrix
        glm::vec4 bounding_center_radius{0.0f}; ///< xyz: Bounding sphere center (world space), w: Radius
        glm::vec4 bounding_box_min{0.0f};       ///< Local or world AABB minimum extents
        glm::vec4 bounding_box_max{0.0f};       ///< Local or world AABB maximum extents

        uint32_t material_id{0}; ///< Material index for bindless texture lookups
        uint32_t mesh_id{0};     ///< Mesh asset identifier
        uint32_t flags{0};       ///< Render state flags (e.g., cast shadows, static/dynamic)
        uint32_t padding{0};     ///< Explicit padding to reach 128-byte alignment
    };

    /**
     * @brief Global camera parameters stored in UBO (std140 layout).
     */
    struct alignas(16) GPUCameraData {
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
        glm::mat4 view_projection{1.0f};

        glm::vec4 camera_position{0.0f}; ///< xyz: Camera position, w: Unused
        glm::vec4 frustum_planes[6]{};   ///< 6 Frustum clipping planes (Ax + By + Cz + D = 0)

        glm::vec2 viewport_size{0.0f}; ///< Screen resolution in pixels
        float near_plane{0.1f};
        float far_plane{1000.0f};
    };

    /**
     * @brief Bounding Box representation for GPU-side culling operations.
     */
    struct GPUAABB {
        glm::vec4 min{-0.5f, -0.5f, -0.5f, 1.0f};
        glm::vec4 max{0.5f, 0.5f, 0.5f, 1.0f};
    };
} // namespace engine
