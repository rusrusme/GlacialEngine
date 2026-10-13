//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/SceneRenderer.hpp"

#include "render/gpu/GPUCuller.hpp"
#include "render/gpu/GPUMeshManager.hpp"
#include "render/gpu/MaterialSystem.hpp"
#include "render/pass/MainOpaquePass.hpp"

namespace engine {
    SceneRenderer::SceneRenderer(ShaderLibrary& shader_lib,
                                 GPUMeshManager& mesh_manager,
                                 MaterialSystem& material_system,
                                 TextureSystem& texture_system)
        : shader_library_(shader_lib),
          mesh_manager_(mesh_manager),
          material_system_(material_system),
          texture_system_(texture_system) {
        ///////////////////////////////////////////////////////////////////////////////////////
        camera_buffer_ = std::make_unique<UniformBuffer>(sizeof(GPUCameraData), 0);
        constexpr size_t initial_instance_capacity = 1024 * sizeof(GPUInstanceData);
        instance_buffer_ = std::make_unique<ShaderStorageBuffer>(initial_instance_capacity, 1);

        constexpr size_t initial_indirect_capacity = 1024 * sizeof(GPUDrawCommand);
        indirect_draw_buffer_ = std::make_unique<IndirectBuffer>(initial_indirect_capacity);

        setup_passes();
    }

    void SceneRenderer::begin_frame(const GPUCameraData& camera) {
        frame_instances_.clear();
        frame_draw_commands_.clear();

        camera_buffer_->set_data(&camera, sizeof(GPUCameraData), 0);
    }

    void SceneRenderer::submit_instance(const GPUInstanceData& instance, uint32_t submesh_index) {
        const uint32_t instance_id = static_cast<uint32_t>(frame_instances_.size());
        frame_instances_.push_back(instance);

        const GPUSubmeshData& submesh_data = mesh_manager_.submesh(submesh_index);

        GPUDrawCommand command{};
        command.count = submesh_data.index_count;
        command.instance_count = 1; // 1 before GPU culling dispatch
        command.first_index = submesh_data.first_index;
        command.base_vertex = submesh_data.base_vertex;
        command.base_instance = instance_id;

        frame_draw_commands_.push_back(command);
    }

    void SceneRenderer::end_frame(CommandBuffer& command, uint32_t frame_index) {
        if (frame_instances_.empty()) {
            return;
        }
        material_system_.flush_to_gpu();
        mesh_manager_.flush_to_gpu();

        const size_t instance_bytes = frame_instances_.size() * sizeof(GPUInstanceData);
        if (!instance_buffer_ || instance_buffer_->size() < instance_bytes) {
            size_t new_cap = instance_buffer_ ? instance_buffer_->size() * 2 : 1024 * sizeof(GPUInstanceData);
            while (new_cap < instance_bytes) {
                new_cap *= 2;
            }
            instance_buffer_ = std::make_unique<ShaderStorageBuffer>(new_cap, 1);
        }
        instance_buffer_->set_data(frame_instances_.data(), instance_bytes, 0);

        const size_t indirect_bytes = frame_draw_commands_.size() * sizeof(GPUDrawCommand);
        if (!indirect_draw_buffer_ || indirect_draw_buffer_->size() < indirect_bytes) {
            size_t new_cap = indirect_draw_buffer_ ? indirect_draw_buffer_->size() * 2 : 1024 * sizeof(GPUDrawCommand);
            while (new_cap < indirect_bytes) {
                new_cap *= 2;
            }
            indirect_draw_buffer_ = std::make_unique<IndirectBuffer>(new_cap);
        }
        indirect_draw_buffer_->set_data(frame_draw_commands_.data(), indirect_bytes, 0);

        render_graph_.execute(command, frame_index);
    }

    void SceneRenderer::setup_passes() {
        render_graph_.add_pass<MainOpaquePass>(
            shader_library_,
            mesh_manager_,
            material_system_,
            texture_system_,
            *camera_buffer_,
            *instance_buffer_,
            *indirect_draw_buffer_
        );
        render_graph_.compile();
    }
}
