//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace engine {
    /**
     * @brief Native indirect draw command structure for GPU-driven rendering.
     * @note Matches the exact 20-byte memory layout required by glMultiDrawElementsIndirect and vkCmdDrawIndexedIndirect.
     * @warning Must NOT be padded: the compute shaders declare the matching std430 struct, whose array stride is
     *          exactly 20 bytes. Any padding here (e.g. alignas(16)) desynchronizes the GPU-side write offsets
     *          from the CPU-side reads, so culling results land in the wrong command slots.
     */
    struct GPUDrawCommand {
        uint32_t count{0};          ///< Number of indices to draw
        uint32_t instance_count{0}; ///< Dynamically toggled by Compute Culling (0 = culled, 1 = visible)
        uint32_t first_index{0};    ///< Byte offset in the Index Buffer / sizeof(IndexType)
        int32_t base_vertex{0};     ///< Offset added to vertex indices
        uint32_t base_instance{0};  ///< Instance ID offset (used for gl_BaseInstance / gl_DrawID lookup)
    };

    static_assert(sizeof(GPUDrawCommand) == 20,
                  "GPUDrawCommand must stay tightly packed (5 x 4 bytes) to match the std430 layout declared in the cull shaders.");

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

    static_assert(sizeof(GPUInstanceData) == 128,
              "GPUInstanceData must be exactly 128 bytes to match std430 SSBO layout.");

    /**
     * @brief Global camera parameters stored in UBO (std140 layout).
     */
    struct alignas(16) GPUCameraData {
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
        glm::mat4 view_projection{1.0f};

        glm::vec4 position{0.0f}; ///< xyz: Camera position, w: Unused
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

    /**
     * @brief Submesh descriptor for GPU-side compute culling and draw command generation (std430 layout).
     * @note Size: 64 bytes aligned to 16 bytes.
     */
    struct alignas(16) GPUSubmeshData {
        glm::vec4 bounding_box_min{-0.5f, -0.5f, -0.5f, 1.0f};
        glm::vec4 bounding_box_max{0.5f, 0.5f, 0.5f, 1.0f};
        glm::vec4 bounding_center_radius{0.0f, 0.0f, 0.0f, 0.5f};

        uint32_t first_index{0}; ///< First index offset in global Index Buffer
        int32_t base_vertex{0};  ///< Base vertex offset in global Vertex Buffer
        uint32_t index_count{0}; ///< Total index count for this submesh
        uint32_t flags{0};       ///< Submesh property flags
    };

    static_assert(sizeof(GPUSubmeshData) == 64,
                  "GPUSubmeshData must be exactly 64 bytes to match std430 SSBO layout.");

    struct alignas(16) GPUMaterialData {
        glm::vec4 albedo_factor{1.0f, 1.0f, 1.0f, 1.0f}; ///< Offset 0 (16 bytes)

        float metallic_factor{1.0f};  ///< Offset 16 (4 bytes)
        float roughness_factor{1.0f}; ///< Offset 20 (4 bytes)
        float normal_scale{1.0f};     ///< Offset 24 (4 bytes)
        float ao_strength{1.0f};      ///< Offset 28 (4 bytes)

        uint32_t albedo_texture_index{0};     ///< Offset 32 (4 bytes)
        uint32_t normal_texture_index{0};     ///< Offset 36 (4 bytes)
        uint32_t metallic_roughness_index{0}; ///< Offset 40 (4 bytes)
        uint32_t ao_texture_index{0};         ///< Offset 44 (4 bytes)

        uint32_t flags{0};            ///< Offset 48 (4 bytes)
        uint32_t padding[3]{0, 0, 0}; ///< Offset 52 (12 bytes)
    };

    static_assert(sizeof(GPUMaterialData) == 64,
                  "GPUMaterialData must be exactly 64 bytes to match std430 SSBO layout.");
} // namespace engine
