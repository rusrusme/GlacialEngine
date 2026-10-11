//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/ComputePipeline.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    class HZBGenerator {
    public:
        HZBGenerator();
        ~HZBGenerator() = default;

        /**
         * @brief Generates the complete Hierarchical Z-Buffer mip chain on the GPU.
         * @param cmd Active CommandBuffer
         * @param depth_texture Source depth buffer (level 0)
         * @param hzb_texture Target HZB texture with complete mip chain allocated
         */
        void generate(CommandBuffer& cmd, const Texture2D& depth_texture, const Texture2D& hzb_texture);

    private:
        Shared<ComputePipeline> hzb_pipeline_;
        DescriptorSet descriptor_set_;
    };
}
