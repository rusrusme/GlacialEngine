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
                      TextureSystem& texture_system);
        ~SceneRenderer() = default;

        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer& operator=(const SceneRenderer&) = delete;
        SceneRenderer(SceneRenderer&&) noexcept = delete;
        SceneRenderer& operator=(SceneRenderer&&) noexcept = delete;

        void begin_frame(const GPUCameraData& camera);
        void submit_instance(const GPUInstanceData& instance, uint32_t submesh_index);
        void end_frame(CommandBuffer& command, uint32_t frame_index);

        [[nodiscard]] RenderGraph& render_graph() { return render_graph_; }

    private:
        void setup_passes();

        ShaderLibrary& shader_library_;
        GPUMeshManager& mesh_manager_;
        MaterialSystem& material_system_;
        TextureSystem& texture_system_;

        RenderGraph render_graph_;

        std::vector<GPUInstanceData> frame_instances_;
        std::vector<GPUDrawCommand> frame_draw_commands_;

        Unique<ShaderStorageBuffer> instance_buffer_{nullptr};
        Unique<IndirectBuffer> indirect_draw_buffer_{nullptr};
        Unique<UniformBuffer> camera_buffer_{nullptr};
    };
}
