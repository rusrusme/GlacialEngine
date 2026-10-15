//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#include <cstdint>
#include <glad/glad.h>

namespace engine {
    class GLIndexBuffer;
    class GLVertexBuffer;
    class GLIndirectBuffer;
    class GLFramebuffer;
    class GLPipelineState;
    class GLDescriptorSet;

    struct ClearColor {
        float r{0.0f}, g{0.0f}, b{0.0f}, a{1.0f};
    };

    struct RenderPassBeginInfo {
        const GLFramebuffer* framebuffer{nullptr};
        ClearColor clear_color{0.0f, 0.0f, 0.0f, 1.0f};
        float clear_depth{1.0f};
        uint32_t clear_stencil{0};
        bool clear_color_target{true};
        bool clear_depth_target{true};
        bool clear_stencil_target{false};
    };

    class GLCommandBuffer {
    public:
        GLCommandBuffer() = default;
        ~GLCommandBuffer() = default;

        GLCommandBuffer(const GLCommandBuffer&) = delete;
        GLCommandBuffer& operator=(const GLCommandBuffer&) = delete;
        GLCommandBuffer(GLCommandBuffer&&) noexcept = default;
        GLCommandBuffer& operator=(GLCommandBuffer&&) noexcept = default;

        void begin() noexcept;
        void end() noexcept;
        void reset() noexcept;

        void dispatch(uint32_t group_x, uint32_t group_y = 1, uint32_t group_z = 1);

        // RenderPass Management
        void begin_render_pass(const GLFramebuffer& framebuffer, const float clear_color[4] = nullptr);
        void begin_render_pass(GLFramebuffer& framebuffer, const float clear_color[4] = nullptr);
        void begin_render_pass(const RenderPassBeginInfo& begin_info);
        void end_render_pass();

        // Pipeline & State Binding
        void bind_pipeline(const GLPipelineState& pipeline);
        void bind_vertex_buffer(uint32_t binding_slot, const GLVertexBuffer& buffer, size_t offset = 0, uint32_t stride = 0);
        void bind_index_buffer(const GLIndexBuffer& buffer, IndexType index_type = IndexType::UInt32);
        void bind_descriptor_set(DescriptorSetIndex set_index, const GLDescriptorSet& descriptor_set);

        // Dynamic States
        void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void set_scissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

        void memory_barrier(MemoryBarrierFlags flags);

        // Draw Calls
        void draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0);
        void draw_indexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t first_instance = 0);
        void draw_indexed_indirect(const GLIndirectBuffer& indirect_buffer, size_t offset = 0, uint32_t draw_count = 1, uint32_t stride = sizeof(DrawIndexedIndirectCommand));

        [[nodiscard]] bool is_recording() const noexcept { return is_recording_; }

    private:
        IndexType current_index_type_{IndexType::UInt32};
        GLenum gl_index_type_{GL_UNSIGNED_INT};
        size_t index_element_size_{4};
        bool is_recording_{false};
    };
} // namespace engine
