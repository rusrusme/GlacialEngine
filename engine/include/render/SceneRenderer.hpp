//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <vector>

#include "gpu/GPUData.hpp"
#include "graph/RenderGraph.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    class GPUMeshManager;
    class MaterialSystem;
    class TextureSystem;

    /**
     * @brief High-level CPU scene submitter and RenderGraph orchestrator.
     */
    class SceneRenderer {
    public:
        SceneRenderer(GPUMeshManager& mesh_mgr, MaterialSystem& mat_sys, TextureSystem& tex_sys);
        ~SceneRenderer() = default;

        void begin_frame(const GPUCameraData& camera);
        void submit_instance(const GPUInstanceData& instance, uint32_t submesh_index);
        void end_frame(CommandBuffer& cmd, uint32_t frame_index);

        [[nodiscard]] RenderGraph& render_graph() { return render_graph_; }

    private:
        void setup_passes();

        GPUMeshManager& mesh_manager_;
        MaterialSystem& material_system_;
        TextureSystem& texture_system_;

        RenderGraph render_graph_;

        std::vector<GPUInstanceData> frame_instances_;
        std::vector<GPUDrawCommand> frame_draw_commands_;

        Unique<ShaderStorageBuffer> instance_buffer_;
        Unique<IndirectBuffer> indirect_draw_buffer_;
        Unique<UniformBuffer> camera_buffer_;
    };
}
