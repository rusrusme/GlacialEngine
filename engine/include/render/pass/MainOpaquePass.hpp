//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/RenderObject.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/Framebuffer.hpp"
#include "render/rhi/PipelineState.hpp"
#include "render/rhi/RHIDevice.hpp"

namespace engine {
    class RenderGraph;

    class MainOpaquePass {
    public:
        MainOpaquePass() = default;
        ~MainOpaquePass() = default;

        void init(const RHIDevice& device);
        void execute(CommandBuffer& cb,
                     Framebuffer& target_framebuffer,
                     const DescriptorSet& frame_descriptor_set,
                     std::span<const RenderObject> objects);

        // TODO Once RenderGraphs are added implement this
        void add_to_graph(RenderGraph& graph,
                          std::span<const RenderObject> objects,
                          const std::string& out_color,
                          const std::string& out_depth);

    private:
        Shared<Shader> shader_{nullptr};
        Unique<PipelineState> pipeline_{nullptr};

        Unique<ShaderStorageBuffer> object_ssbo_{nullptr};
        Unique<DescriptorSet> pass_descriptor_set_{nullptr};
        Unique<Texture2D> default_white_texture_{nullptr};
        size_t max_objects_{1024};
    };
}
