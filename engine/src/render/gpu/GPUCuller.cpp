//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/gpu/GPUCuller.hpp"

#include <memory>

#include "core/Log.hpp"
#include "render/rhi/Shader.hpp"

namespace engine {
    GPUCuller::GPUCuller() {
        // Frustum Culling Pipeline
        {
            ComputePipelineDesc desc;
            desc.shader = std::make_shared<Shader>("assets/shaders/compiled/gpu/frustum_cull.bshb");

            if (desc.shader && desc.shader->renderer_id() != 0) {
                frustum_cull_pipeline_ = std::make_shared<ComputePipeline>(desc);
            }
            else {
                CORE_ERROR("[GPUCuller] Failed to initialize frustum culling compute pipeline shader");
            }
        }

        // HZB Occlusion Culling Pipeline
        {
            ComputePipelineDesc desc;
            desc.shader = std::make_shared<Shader>("assets/shaders/compiled/gpu/occlusion_cull.bshb");

            if (desc.shader && desc.shader->renderer_id() != 0) {
                hzb_cull_pipeline_ = std::make_shared<ComputePipeline>(desc);
            }
            else {
                CORE_WARN("[GPUCuller] Occlusion culling shader not found or failed to compile. Fallback to Frustum Culling only.");
            }
        }
        CORE_INFO("[GPUCuller] Initialized GPU Culling Pipeline successfully");
    }

    void GPUCuller::dispatch_frustum_cull(CommandBuffer& cmd, uint32_t total_instances, const ShaderStorageBuffer& instance_buffer, const IndirectBuffer& indirect_draw_buffer, const UniformBuffer& camera_buffer) {
        if (total_instances == 0 || !frustum_cull_pipeline_) return;

        descriptor_set_.write_uniform_buffer(0, camera_buffer);
        descriptor_set_.write_storage_buffer(0, instance_buffer);
        descriptor_set_.write_storage_buffer(1, indirect_draw_buffer);
        descriptor_set_.bind(DescriptorSetIndex::Pass);

        frustum_cull_pipeline_->bind();
        const uint32_t workgroup_x = (total_instances + 63) / 64;
        cmd.dispatch(workgroup_x, 1, 1);

        ComputePipeline::set_memory_barrier(MemoryBarrierFlags::CommandData | MemoryBarrierFlags::ShaderStorage);
    }

    void GPUCuller::dispatch_occlusion_cull(CommandBuffer& cmd, uint32_t total_instances, const ShaderStorageBuffer& instance_buffer, const IndirectBuffer& indirect_draw_buffer, const UniformBuffer& camera_buffer, const Texture2D& hzb_texture) {
        if (total_instances == 0 || !hzb_cull_pipeline_) return;

        descriptor_set_.write_uniform_buffer(0, camera_buffer);
        descriptor_set_.write_storage_buffer(0, instance_buffer);
        descriptor_set_.write_storage_buffer(1, indirect_draw_buffer);
        descriptor_set_.write_texture(0, hzb_texture);
        descriptor_set_.bind(DescriptorSetIndex::Pass);

        hzb_cull_pipeline_->bind();
        const uint32_t workgroup_x = (total_instances + 63) / 64;
        cmd.dispatch(workgroup_x, 1, 1);

        ComputePipeline::set_memory_barrier(MemoryBarrierFlags::CommandData | MemoryBarrierFlags::ShaderStorage);
    }
}
