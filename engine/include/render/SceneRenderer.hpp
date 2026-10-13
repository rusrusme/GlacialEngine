//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <vector>

#include "gpu/GPUData.hpp"
#include "graph/RenderGraph.hpp"
#include "render/gpu/GPUCuller.hpp"
#include "render/gpu/HZBGenerator.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    class ShaderLibrary;
    class GPUMeshManager;
    class MaterialSystem;
    class TextureSystem;

    /**
     * @brief High-level CPU scene submitter and RenderGraph orchestrator.
     */
    class SceneRenderer {
    public:
        SceneRenderer(ShaderLibrary& shader_lib,
                      GPUMeshManager& mesh_manager,
                      MaterialSystem& material_system,
                      TextureSystem& texture_system,
                      uint32_t viewport_width = 1920,
                      uint32_t viewport_height = 1080);
        ~SceneRenderer() = default;

        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer& operator=(const SceneRenderer&) = delete;
        SceneRenderer(SceneRenderer&&) noexcept = delete;
        SceneRenderer& operator=(SceneRenderer&&) noexcept = delete;

        void begin_scene(const GPUCameraData& camera);
        void submit_instance(const GPUInstanceData& instance, uint32_t submesh_index);
        void end_scene();

        void resize_viewport(uint32_t width, uint32_t height);
        [[nodiscard]] Texture2D* final_color_target() const { return main_color_target_.get(); }

        [[nodiscard]] RenderGraph& render_graph() { return render_graph_; }

    private:
        void setup_passes();
        void recreate_viewport_targets(uint32_t width, uint32_t height);

        ShaderLibrary& shader_library_;
        GPUMeshManager& mesh_manager_;
        MaterialSystem& material_system_;
        TextureSystem& texture_system_;

        RenderGraph render_graph_;

        uint32_t viewport_width_{1920};
        uint32_t viewport_height_{1080};

        Unique<Texture2D> main_color_target_{nullptr};
        Unique<Texture2D> main_depth_target_{nullptr};
        Unique<Texture2D> hzb_texture_{nullptr};

        Unique<GPUCuller> gpu_culler_{nullptr};
        Unique<HZBGenerator> hzb_generator_{nullptr};

        Vector<GPUInstanceData> frame_instances_;
        Vector<GPUDrawCommand> frame_draw_commands_;

        Unique<ShaderStorageBuffer> instance_buffer_{nullptr};
        Unique<IndirectBuffer> indirect_draw_buffer_{nullptr};
        Unique<UniformBuffer> camera_buffer_{nullptr};
    };
}
