//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/gpu/GPUMeshManager.hpp"

#include <cassert>
#include <glm/geometric.hpp>

namespace engine {
    GPUMeshManager::GPUMeshManager(size_t vertex_buffer_size, size_t index_buffer_size) {
        global_vbo_ = std::make_unique<VertexBuffer>(vertex_buffer_size);
        global_ibo_ = std::make_unique<IndexBuffer>(index_buffer_size);

        constexpr size_t initial_ssbo_capacity = 256 * sizeof(GPUSubmeshData);
        submesh_ssbo_ = std::make_unique<ShaderStorageBuffer>(initial_ssbo_capacity, 0);
    }

    GPUMeshAllocation GPUMeshManager::register_submesh(const void* vertex_data, size_t vertex_bytes, uint32_t vertex_count, const uint32_t* index_data, uint32_t index_count, const GPUAABB& bounds) {
        assert(vertex_data && index_data && "Vertex and Index data must not be null");

        const size_t vertex_stride = vertex_count > 0 ? vertex_bytes / vertex_count : 0;
        const size_t vbo_byte_offset = current_vertex_offset_ * vertex_stride;
        const size_t ibo_byte_offset = current_index_offset_ * sizeof(uint32_t);

        global_vbo_->set_data(vertex_data, vertex_bytes, vbo_byte_offset);
        global_ibo_->set_data(index_data, index_count * sizeof(uint32_t), ibo_byte_offset);

        const glm::vec3 center = glm::vec3(bounds.min + bounds.max) * 0.5f;
        const float radius = glm::length(glm::vec3(bounds.max - bounds.min) * 0.5f);

        GPUSubmeshData submesh_data{};
        submesh_data.bounding_box_min = bounds.min;
        submesh_data.bounding_box_max = bounds.max;
        submesh_data.bounding_center_radius = glm::vec4(center, radius);

        submesh_data.first_index = current_index_offset_;
        submesh_data.base_vertex = static_cast<int32_t>(current_vertex_offset_);
        submesh_data.index_count = index_count;
        submesh_data.flags = 0;

        const auto mesh_id = static_cast<uint32_t>(submeshes_.size());
        submeshes_.push_back(submesh_data);

        GPUMeshAllocation allocation{};
        allocation.mesh_id = mesh_id;
        allocation.first_index = current_index_offset_;
        allocation.base_vertex = static_cast<int32_t>(current_vertex_offset_);
        allocation.index_count = index_count;
        allocation.vertex_count = vertex_count;

        current_vertex_offset_ += vertex_count;
        current_index_offset_ += index_count;
        is_dirty_ = true;

        return allocation;
    }

    void GPUMeshManager::flush_to_gpu() {
        if (!is_dirty_ || submeshes_.empty()) {
            return;
        }
        const size_t required_bytes = submeshes_.size() * sizeof(GPUSubmeshData);

        if (!submesh_ssbo_ || submesh_ssbo_->size() < required_bytes) {
            size_t new_capacity = submesh_ssbo_ ? submesh_ssbo_->size() * 2 : 256 * sizeof(GPUSubmeshData);
            while (new_capacity < required_bytes) {
                new_capacity *= 2;
            }
            submesh_ssbo_ = std::make_unique<ShaderStorageBuffer>(new_capacity, 0);
        }

        submesh_ssbo_->set_data(submeshes_.data(), required_bytes);
        is_dirty_ = false;
    }
}
