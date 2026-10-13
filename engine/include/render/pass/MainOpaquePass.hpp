//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/graph/RenderPass.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/PipelineState.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    class ShaderLibrary;
    class GPUMeshManager;
    class MaterialSystem;
    class TextureSystem;

    class MainOpaquePass : public RenderPass {
    public:
        MainOpaquePass(ShaderLibrary& shader_library,
                       GPUMeshManager& mesh_manager,
                       MaterialSystem& material_system,
                       TextureSystem& texture_system,
                       UniformBuffer& camera_buffer,
                       ShaderStorageBuffer& instance_buffer,
                       IndirectBuffer& indirect_buffer);

        void setup(RenderGraphBuilder& builder) override;
        void execute(const RenderContext& context, const RenderGraphRegistry& registry) override;

    private:
        GPUMeshManager& mesh_manager_;
        MaterialSystem& material_system_;
        TextureSystem& texture_system_;

        UniformBuffer& camera_buffer_;
        ShaderStorageBuffer& instance_buffer_;
        IndirectBuffer& indirect_draw_buffer_;

        Shared<PipelineState> pipeline_state_;
        Unique<DescriptorSet> descriptor_set_;
    };
}
