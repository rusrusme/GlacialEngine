//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "render/gpu/GPUData.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/ComputePipeline.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    class GPUCuller {
    public:
        GPUCuller();
        ~GPUCuller() = default;

        void dispatch_frustum_cull(CommandBuffer& cmd,
                                   uint32_t total_instances,
                                   const ShaderStorageBuffer& instance_buffer,
                                   const IndirectBuffer& indirect_draw_buffer,
                                   const UniformBuffer& camera_buffer);

        void dispatch_occlusion_cull(CommandBuffer& cmd,
                                     uint32_t total_instances,
                                     const ShaderStorageBuffer& instance_buffer,
                                     const IndirectBuffer& indirect_draw_buffer,
                                     const UniformBuffer& camera_buffer,
                                     const Texture2D& hzb_texture);

    private:
        Shared<ComputePipeline> frustum_cull_pipeline_;
        Shared<ComputePipeline> hzb_cull_pipeline_;
        DescriptorSet descriptor_set_;
    };
}
