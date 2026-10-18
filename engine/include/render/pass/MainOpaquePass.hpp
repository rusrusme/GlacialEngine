//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/graph/RenderPass.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/Framebuffer.hpp"
#include "render/rhi/PipelineState.hpp"
#include "render/rhi/RHIDevice.hpp"

namespace engine {
    class Mesh;

    struct RenderObject {
        const Mesh* mesh{nullptr};
        glm::mat4 transform{1.0f};
    };

    class MainOpaquePass {
    public:
        MainOpaquePass() = default;
        ~MainOpaquePass() = default;

        void init(const RHIDevice& device);
        void execute(CommandBuffer& cb,
                     Framebuffer& target_framebuffer,
                     const DescriptorSet& frame_descriptor_set,
                     std::span<const RenderObject> objects);

    private:
        Shared<Shader> shader_{nullptr};
        Unique<PipelineState> pipeline_{nullptr};

        Unique<ShaderStorageBuffer> object_ssbo_{nullptr};
        Unique<DescriptorSet> pass_descriptor_set_{nullptr};
        size_t max_objects_{1024};
    };
}
