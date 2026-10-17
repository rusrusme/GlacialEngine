//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/pass/MainOpaquePass.hpp"

#include <memory>

#include "render/gpu/GPUMeshManager.hpp"
#include "render/gpu/MaterialSystem.hpp"
#include "render/gpu/TextureSystem.hpp"
#include "render/graph/RenderGraph.hpp"
#include "render/rhi/PipelineSpecs.hpp"
#include "render/rhi/Shader.hpp"
#include "render/shader/ShaderLibrary.hpp"

namespace engine {
    MainOpaquePass::MainOpaquePass(ShaderLibrary& shader_library,
                                   GPUMeshManager& mesh_manager,
                                   MaterialSystem& material_system,
                                   TextureSystem& texture_system,
                                   UniformBuffer& camera_buffer,
                                   ShaderStorageBuffer& instance_buffer,
                                   IndirectBuffer& indirect_buffer)
        : RenderPass("Main Opaque Pass"),
          mesh_manager_(mesh_manager),
          material_system_(material_system),
          texture_system_(texture_system),
          camera_buffer_(camera_buffer),
          instance_buffer_(instance_buffer),
          indirect_draw_buffer_(indirect_buffer) {
        //////////////////////////////////////////////////////////
        const Shared<Shader> mesh_shader = shader_library.exists("mesh")
                                               ? shader_library.get("mesh")
                                               : shader_library.load("mesh", "assets/shaders/compiled/mesh.bshb");

        PipelineStateDesc pipeline_desc{};
        pipeline_desc.shader = mesh_shader;

        pipeline_desc.vertex_layout = {
            {ShaderDataType::Float3, "a_position"},
            {ShaderDataType::Float3, "a_normal"},
            {ShaderDataType::Float2, "a_uv"},
            {ShaderDataType::Float4, "a_tangent"}
        };

        pipeline_desc.cull_mode = CullMode::None;
        pipeline_desc.polygon_mode = PolygonMode::Fill;
        pipeline_desc.depth_test = false;
        pipeline_desc.depth_write = true;
        pipeline_desc.depth_compare = CompareOp::Less;
        pipeline_desc.blend_enable = false;

        pipeline_state_ = std::make_shared<PipelineState>(pipeline_desc);

        descriptor_set_ = std::make_unique<DescriptorSet>();

        descriptor_set_->write_uniform_buffer(0, camera_buffer_);   // Camera UBO (Slot 0)
        descriptor_set_->write_storage_buffer(1, instance_buffer_); // Instance SSBO (Slot 1)

        if (mesh_manager_.submesh_ssbo()) {
            descriptor_set_->write_storage_buffer(2, *mesh_manager_.submesh_ssbo()); // Submesh SSBO (Slot 2)
        }

        material_system_.bind(*descriptor_set_, 3); // Material SSBO (Slot 3)
        texture_system_.bind(*descriptor_set_, 4);  // Texture Array Sampler (Slot 4)

        descriptor_set_->update();
    }

    void MainOpaquePass::setup(RenderGraphBuilder& builder) {
        builder.write_texture("MainColorTarget");
        builder.write_texture("MainDepthTarget");
    }

    // void MainOpaquePass::execute(const RenderContext& context, const RenderGraphRegistry& registry) {
    //     CommandBuffer& cmd = context.command_buffer;
    //
    //     cmd.bind_pipeline(*pipeline_state_);
    //     // TODO I removed the set_index Frame -> Needs to called somewhere else
    //     cmd.bind_descriptor_set(*descriptor_set_);
    //
    //     if (mesh_manager_.vertex_buffer()) {
    //         cmd.bind_vertex_buffer(0, *mesh_manager_.vertex_buffer());
    //     }
    //     if (mesh_manager_.index_buffer()) {
    //         cmd.bind_index_buffer(*mesh_manager_.index_buffer());
    //     }
    //
    //     cmd.draw_indexed_indirect(indirect_draw_buffer_);
    // }
}
