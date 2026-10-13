//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/gpu/HZBGenerator.hpp"

#include <algorithm>

#include "core/Log.hpp"
#include "render/rhi/Shader.hpp"

namespace engine {
    HZBGenerator::HZBGenerator() {
        {
            ComputePipelineDesc desc;
            desc.shader = std::make_shared<Shader>("assets/shaders/compiled/gpu/hzb_initial.bshb");
            if (desc.shader && desc.shader->renderer_id() != 0) {
                initial_pipeline_ = std::make_shared<ComputePipeline>(desc);
            }
            else {
                CORE_ERROR("[HZBGenerator] Failed to initialize hzb_initial compute pipeline");
            }
        }

        {
            ComputePipelineDesc desc;
            desc.shader = std::make_shared<Shader>("assets/shaders/compiled/gpu/hzb_reduce.bshb");
            if (desc.shader && desc.shader->renderer_id() != 0) {
                reduce_pipeline_ = std::make_shared<ComputePipeline>(desc);
            }
            else {
                CORE_ERROR("[HZBGenerator] Failed to initialize hzb_reduce compute pipeline");
            }
        }

        CORE_INFO("[HZBGenerator] Initialized HZB Generator successfully");
    }

    void HZBGenerator::generate(CommandBuffer& cmd, const Texture2D& depth_texture, const Texture2D& hzb_texture) {
        const uint32_t mip_levels = hzb_texture.mip_levels();
        if (mip_levels == 0 || !initial_pipeline_ || !reduce_pipeline_) {
            return;
        }

        uint32_t current_width = hzb_texture.width();
        uint32_t current_height = hzb_texture.height();

        // 1. Initial Pass: Sample source depth (sampler2D) -> Write HZB Level 0 (image2D)
        initial_pipeline_->bind();
        descriptor_set_.write_texture(0, depth_texture);
        descriptor_set_.write_storage_image(0, hzb_texture, 0, ImageAccess::WriteOnly);
        descriptor_set_.bind(DescriptorSetIndex::Pass);

        cmd.dispatch((current_width + 15) / 16, (current_height + 15) / 16, 1);
        ComputePipeline::set_memory_barrier(MemoryBarrierFlags::TextureFetch | MemoryBarrierFlags::ShaderStorage);

        // 2. Reduction Passes: Read HZB Level k-1 (image2D) -> Write HZB Level k (image2D)
        reduce_pipeline_->bind();
        for (uint32_t level = 1; level < mip_levels; ++level) {
            const uint32_t next_width = std::max(1u, current_width / 2);
            const uint32_t next_height = std::max(1u, current_height / 2);

            descriptor_set_.write_storage_image(0, hzb_texture, level - 1, ImageAccess::ReadOnly);
            descriptor_set_.write_storage_image(1, hzb_texture, level, ImageAccess::WriteOnly);
            descriptor_set_.bind(DescriptorSetIndex::Pass);

            cmd.dispatch((next_width + 15) / 16, (next_height + 15) / 16, 1);
            ComputePipeline::set_memory_barrier(MemoryBarrierFlags::TextureFetch | MemoryBarrierFlags::ShaderStorage);

            current_width = next_width;
            current_height = next_height;
        }
    }
}
