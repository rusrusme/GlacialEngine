//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <vector>

#include "GPUData.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    /**
     * @brief Allocation metadata returned when registering a mesh/submesh into global geometry mega-buffers.
     */
    struct GPUMeshAllocation {
        uint32_t mesh_id{0};
        uint32_t first_index{0};
        int32_t base_vertex{0};
        uint32_t index_count{0};
        uint32_t vertex_count{0};
    };

    /**
     * @brief Consolidated global mesh management backing geometry and submesh descriptor SSBOs.
     */
    class GPUMeshManager {
    public:
        GPUMeshManager(size_t vertex_buffer_size, size_t index_buffer_size);
        ~GPUMeshManager() = default;

        /**
         * @brief Registers raw geometry into global VBO/IBO mega-buffers and records GPUSubmeshData descriptor.
         */
        GPUMeshAllocation register_submesh(const void* vertex_data, size_t vertex_bytes, uint32_t vertex_count,
                                           const uint32_t* index_data, uint32_t index_count,
                                           const GPUAABB& bounds);

        /**
         * @brief Uploads updated submesh descriptors to the GPU Submesh SSBO.
         */
        void flush_to_gpu();

        [[nodiscard]] VertexBuffer* vertex_buffer() const { return global_vbo_.get(); }
        [[nodiscard]] IndexBuffer* index_buffer() const { return global_ibo_.get(); }
        [[nodiscard]] ShaderStorageBuffer* submesh_ssbo() const { return submesh_ssbo_.get(); }

    private:
        Unique<VertexBuffer> global_vbo_{nullptr};
        Unique<IndexBuffer> global_ibo_{nullptr};
        Unique<ShaderStorageBuffer> submesh_ssbo_{nullptr};

        std::vector<GPUSubmeshData> submeshes_;

        uint32_t current_vertex_offset_{0};
        uint32_t current_index_offset_{0};
        bool is_dirty_{false};
    };
}
