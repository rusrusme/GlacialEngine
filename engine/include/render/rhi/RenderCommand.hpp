//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/Buffer.hpp"
#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/Framebuffer.hpp"
#include "render/rhi/PipelineState.hpp"
#include "render/rhi/RendererAPI.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    class RenderCommand {
    public:
        static void init() { renderer_api_.init(); }
        static void shutdown() { renderer_api_.shutdown(); }

        static void begin_render_pass(CommandBuffer& command_buffer, Framebuffer& framebuffer, const float clear_color[4]) {
            command_buffer.begin_render_pass(framebuffer, clear_color);
        }

        static void end_render_pass(CommandBuffer& command_buffer) {
            command_buffer.end_render_pass();
        }

        static void bind_pipeline(CommandBuffer& command_buffer, const PipelineState& pipeline) {
            command_buffer.bind_pipeline(pipeline);
        }

        static void bind_vertex_buffer(CommandBuffer& command_buffer, uint32_t binding_slot, const VertexBuffer& buffer, size_t offset = 0) {
            command_buffer.bind_vertex_buffer(binding_slot, buffer, offset);
        }

        static void bind_index_buffer(CommandBuffer& command_buffer, const IndexBuffer& buffer, IndexType index_type = IndexType::UInt32) {
            command_buffer.bind_index_buffer(buffer, index_type);
        }

        static void bind_descriptor_set(CommandBuffer& command_buffer, DescriptorSetIndex set_index, const DescriptorSet& descriptor_set) {
            command_buffer.bind_descriptor_set(set_index, descriptor_set);
        }

        static void set_viewport(CommandBuffer& command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
            command_buffer.set_viewport(x, y, width, height);
        }

        static void draw_indexed(CommandBuffer& command_buffer, uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0) {
            command_buffer.draw_indexed(index_count, instance_count, first_index, vertex_offset);
        }

        static void draw_indexed_indirect(CommandBuffer& command_buffer, const IndirectBuffer& indirect_buffer, size_t offset = 0, uint32_t draw_count = 1) {
            command_buffer.draw_indexed_indirect(indirect_buffer, offset, draw_count);
        }

    private:
        inline static RendererAPI renderer_api_{};
    };
}
