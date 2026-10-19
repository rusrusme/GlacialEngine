//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/platform/opengl/GLBuffer.hpp"
#include "render/platform/opengl/GLDescriptorSet.hpp"
#include "render/platform/opengl/GLFramebuffer.hpp"
#include "render/platform/opengl/GLPipelineState.hpp"
#include "render/platform/opengl/GLSwapchain.hpp"
#include "render/platform/opengl/GLTexture.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    class GLDevice {
    public:
        GLDevice() = default;
        ~GLDevice() = default;

        void init();
        void shutdown();

        [[nodiscard]] Unique<GLVertexBuffer> create_vertex_buffer(size_t size, const void* data) const;
        [[nodiscard]] Unique<GLIndexBuffer> create_index_buffer(size_t size, const uint32_t* indices) const;
        [[nodiscard]] Unique<GLUniformBuffer> create_uniform_buffer(size_t size, uint32_t binding) const;
        [[nodiscard]] Unique<GLShaderStorageBuffer> create_shader_storage_buffer(size_t size, uint32_t binding) const;
        [[nodiscard]] Unique<GLIndirectBuffer> create_indirect_buffer(size_t size) const;
        [[nodiscard]] Unique<GLPipelineState> create_pipeline_state(const PipelineStateDesc& desc) const;
        [[nodiscard]] Unique<GLFramebuffer> create_framebuffer(const FramebufferSpecification& spec) const;
        [[nodiscard]] Shared<GLSwapchain> create_swapchain(const SwapchainSpec& spec) const;

        [[nodiscard]] Unique<GLDescriptorSet> create_descriptor_set() const;
        [[nodiscard]] Unique<GLTexture2D> create_texture_2d(const TextureSpecification& spec, const void* data) const;

        [[nodiscard]] static constexpr RendererBackend backend() noexcept { return RendererBackend::OpenGL; }
    };
}
