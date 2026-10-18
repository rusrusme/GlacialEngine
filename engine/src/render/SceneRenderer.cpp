//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/SceneRenderer.hpp"

#include "core/Log.hpp"
#include "render/gpu/GPUMeshManager.hpp"
#include "render/gpu/MaterialSystem.hpp"
#include "render/pass/MainOpaquePass.hpp"
#include "render/rhi/RenderContext.hpp"

namespace engine {
    SceneRenderer::SceneRenderer(ShaderLibrary& shader_lib,
                                 GPUMeshManager& mesh_manager,
                                 MaterialSystem& material_system,
                                 TextureSystem& texture_system,
                                 uint32_t viewport_width,
                                 uint32_t viewport_height)
        : shader_library_(shader_lib),
          mesh_manager_(mesh_manager),
          material_system_(material_system),
          texture_system_(texture_system),
          viewport_width_(viewport_width),
          viewport_height_(viewport_height) {
        ///////////////////////////////////////////////////////////////////////////////////////
        camera_buffer_ = std::make_unique<UniformBuffer>(sizeof(GPUCameraData), 0);
        instance_buffer_ = std::make_unique<ShaderStorageBuffer>(1024 * sizeof(GPUInstanceData), 1);
        indirect_draw_buffer_ = std::make_unique<IndirectBuffer>(1024 * sizeof(GPUDrawCommand));

        gpu_culler_ = std::make_unique<GPUCuller>();
        hzb_generator_ = std::make_unique<HZBGenerator>();

        recreate_viewport_targets(viewport_width_, viewport_height_);
        setup_passes();
    }

    void SceneRenderer::begin_scene(const GPUCameraData& camera) {
        frame_instances_.clear();
        frame_draw_commands_.clear();

        camera_buffer_->set_data(&camera, sizeof(GPUCameraData), 0);
    }

    void SceneRenderer::submit_instance(const GPUInstanceData& instance, uint32_t submesh_index) {
        const auto instance_id = static_cast<uint32_t>(frame_instances_.size());
        frame_instances_.push_back(instance);

        const GPUSubmeshData& submesh_data = mesh_manager_.submesh(submesh_index);

        GPUDrawCommand command{};
        command.count = submesh_data.index_count;
        command.instance_count = 1;
        command.first_index = submesh_data.first_index;
        command.base_vertex = submesh_data.base_vertex;
        command.base_instance = instance_id;

        frame_draw_commands_.push_back(command);
    }

    void SceneRenderer::end_scene() {
        if (frame_instances_.empty()) {
            return;
        }

        // CommandBuffer& cmd = RenderContext::active_command_buffer();
        // const uint32_t frame_index = RenderContext::current_frame_index();

        material_system_.flush_to_gpu();
        mesh_manager_.flush_to_gpu();

        const size_t instance_bytes = frame_instances_.size() * sizeof(GPUInstanceData);
        if (!instance_buffer_ || instance_buffer_->size() < instance_bytes) {
            instance_buffer_ = std::make_unique<ShaderStorageBuffer>(instance_bytes * 2, 1);
        }
        instance_buffer_->set_data(frame_instances_.data(), instance_bytes, 0);

        const size_t indirect_bytes = frame_draw_commands_.size() * sizeof(GPUDrawCommand);
        if (!indirect_draw_buffer_ || indirect_draw_buffer_->size() < indirect_bytes) {
            indirect_draw_buffer_ = std::make_unique<IndirectBuffer>(indirect_bytes * 2);
        }
        indirect_draw_buffer_->set_data(frame_draw_commands_.data(), indirect_bytes, 0);

        const auto total_instances = static_cast<uint32_t>(frame_instances_.size());

        // if (main_depth_target_ && hzb_texture_) {
        //     hzb_generator_->generate(cmd, *main_depth_target_, *hzb_texture_);
        //     gpu_culler_->dispatch_occlusion_cull(cmd, total_instances, *instance_buffer_, *indirect_draw_buffer_, *camera_buffer_, *hzb_texture_);
        // }
        // else {
        //     gpu_culler_->dispatch_frustum_cull(cmd, total_instances, *instance_buffer_, *indirect_draw_buffer_, *camera_buffer_);
        // }
        //
        // cmd.memory_barrier(MemoryBarrierFlags::CommandData | MemoryBarrierFlags::ShaderStorage);
        //
        // render_graph_.execute(cmd, frame_index);
    }

    void SceneRenderer::resize_viewport(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0 || (width == viewport_width_ && height == viewport_height_)) {
            return;
        }

        viewport_width_ = width;
        viewport_height_ = height;

        recreate_viewport_targets(viewport_width_, viewport_height_);
    }

    void SceneRenderer::setup_passes() {
        // render_graph_.add_pass<MainOpaquePass>(
        //     shader_library_,
        //     mesh_manager_,
        //     material_system_,
        //     texture_system_,
        //     *camera_buffer_,
        //     *instance_buffer_,
        //     *indirect_draw_buffer_
        // );
        // render_graph_.compile();
    }

    void SceneRenderer::recreate_viewport_targets(uint32_t width, uint32_t height) {
        TextureSpecification color_spec{};
        color_spec.width = width;
        color_spec.height = height;
        color_spec.format = TextureFormat::RGBA8;
        main_color_target_ = std::make_unique<Texture2D>(color_spec);

        TextureSpecification depth_spec{};
        depth_spec.width = width;
        depth_spec.height = height;
        depth_spec.format = TextureFormat::Depth24Stencil8;
        main_depth_target_ = std::make_unique<Texture2D>(depth_spec);

        const auto hzb_width = static_cast<uint32_t>(std::pow(2, std::ceil(std::log2(width))));
        const auto hzb_height = static_cast<uint32_t>(std::pow(2, std::ceil(std::log2(height))));

        TextureSpecification hzb_spec{};
        hzb_spec.width = hzb_width;
        hzb_spec.height = hzb_height;
        hzb_spec.format = TextureFormat::R32F;
        hzb_spec.generate_mipmaps = true;
        hzb_texture_ = std::make_unique<Texture2D>(hzb_spec);

        render_graph_.import_texture("MainColorTarget", main_color_target_.get());
        render_graph_.import_texture("MainDepthTarget", main_depth_target_.get());
    }
}
