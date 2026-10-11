//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/gpu/HZBGenerator.hpp"

#include <algorithm>

#include "core/Log.hpp"
#include "render/rhi/ComputePipelineDesc.hpp"
#include "render/rhi/Shader.hpp"

namespace engine {
    HZBGenerator::HZBGenerator() {
        ComputePipelineDesc desc;
        desc.shader = std::make_shared<Shader>("assets/shaders/compiled/gpu/hzb_generate.bshb");

        if (!desc.shader || desc.shader->renderer_id() == 0) {
            CORE_ERROR("[HZBGenerator] Failed to initialize HZB generation compute pipeline shader");
            return;
        }

        hzb_pipeline_ = std::make_shared<ComputePipeline>(desc);
        CORE_INFO("[HZBGenerator] Initialized HZB Generator successfully");
    }

    void HZBGenerator::generate(CommandBuffer& cmd, const Texture2D& depth_texture, const Texture2D& hzb_texture) {
        if (!hzb_pipeline_) {
            CORE_ERROR("[HZBGenerator] Cannot generate HZB: Compute pipeline not initialized");
            return;
        }

        const uint32_t mip_levels = hzb_texture.mip_levels();
        if (mip_levels <= 1) {
            return;
        }

        hzb_pipeline_->bind();

        uint32_t current_width = hzb_texture.width();
        uint32_t current_height = hzb_texture.height();

        // Iteratively downsample level k-1 into level k
        for (uint32_t level = 1; level < mip_levels; ++level) {
            const uint32_t next_width = std::max(1u, current_width / 2);
            const uint32_t next_height = std::max(1u, current_height / 2);

            // Level 1 reads from source depth_texture (Level 0), subsequent levels read from hzb_texture (Level k-1)
            if (level == 1) {
                descriptor_set_.write_storage_image(0, depth_texture, 0, ImageAccess::ReadOnly);
            }
            else {
                descriptor_set_.write_storage_image(0, hzb_texture, level - 1, ImageAccess::ReadOnly);
            }

            descriptor_set_.write_storage_image(1, hzb_texture, level, ImageAccess::WriteOnly);
            descriptor_set_.bind(DescriptorSetIndex::Pass);

            // Dispatch 16x16 workgroups
            const uint32_t workgroup_x = (next_width + 15) / 16;
            const uint32_t workgroup_y = (next_height + 15) / 16;

            cmd.dispatch(workgroup_x, workgroup_y, 1);

            // Barrier: Ensure level k write completes before level k+1 reads it as source image
            ComputePipeline::set_memory_barrier(MemoryBarrierFlags::TextureFetch | MemoryBarrierFlags::ShaderStorage);

            current_width = next_width;
            current_height = next_height;
        }
    }
}
