//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/gpu/GPUCuller.hpp"

#include <memory>

#include "core/Log.hpp"
#include "render/rhi/ComputePipelineDesc.hpp"
#include "render/rhi/Shader.hpp"

namespace engine {
    GPUCuller::GPUCuller() {
        ComputePipelineDesc desc;
        desc.shader = std::make_shared<Shader>("assets/shaders/gpu/compute/frustum_cull.bshb");

        if (!desc.shader || desc.shader->renderer_id() == 0) {
            CORE_ERROR("[GPUCuller] Failed to initialize frustum culling compute pipeline shader");
            return;
        }
        frustum_cull_pipeline_ = std::make_shared<ComputePipeline>(desc);
        CORE_INFO("[GPUCuller] Initialized GPU Culling Pipeline successfully");
    }

    void GPUCuller::dispatch_cull(CommandBuffer& cmd,
                                  uint32_t total_instances,
                                  const ShaderStorageBuffer& instance_buffer,
                                  const IndirectBuffer& indirect_draw_buffer,
                                  const UniformBuffer& camera_buffer,
                                  const Texture2D* hzb_texture) {
        if (total_instances == 0) {
            return;
        }

        Shared<ComputePipeline> active_pipeline = frustum_cull_pipeline_;
        if (hzb_texture && hzb_cull_pipeline_) {
            active_pipeline = hzb_cull_pipeline_;
        }
        if (!active_pipeline) {
            CORE_ERROR("[GPUCuller] Cannot dispatch culling: No valid compute pipeline bound");
            return;
        }

        descriptor_set_.write_uniform_buffer(0, camera_buffer);
        descriptor_set_.write_storage_buffer(0, instance_buffer);
        descriptor_set_.write_storage_buffer(1, indirect_draw_buffer);

        if (hzb_texture) {
            descriptor_set_.write_texture(0, *hzb_texture);
        }

        descriptor_set_.bind(DescriptorSetIndex::Pass);

        active_pipeline->bind();
        const uint32_t workgroup_x = (total_instances + 63) / 64;
        cmd.dispatch(workgroup_x, 1, 1);

        ComputePipeline::set_memory_barrier(MemoryBarrierFlags::CommandData | MemoryBarrierFlags::ShaderStorage);
    }
}
