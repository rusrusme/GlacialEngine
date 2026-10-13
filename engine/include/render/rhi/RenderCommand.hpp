//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <array>
#include <memory>

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
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;

        static void init() {
            renderer_api_.init();
            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                command_buffers_[i] = std::make_unique<CommandBuffer>();
            }
        }

        static void shutdown() {
            for (auto& cb : command_buffers_) {
                cb.reset();
            }
            renderer_api_.shutdown();
        }

        static void begin_frame() {
            current_frame_index_ = (current_frame_index_ + 1) % MAX_FRAMES_IN_FLIGHT;
            command_buffers_[current_frame_index_]->begin();
        }

        static void end_frame() {
            command_buffers_[current_frame_index_]->end();
        }

        [[nodiscard]] static CommandBuffer& active_command_buffer() {
            return *command_buffers_[current_frame_index_];
        }

        [[nodiscard]] static uint32_t current_frame_index() noexcept {
            return current_frame_index_;
        }

        // --- Pass & Binding Delegation ---
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

        inline static std::array<Unique<CommandBuffer>, MAX_FRAMES_IN_FLIGHT> command_buffers_{};
        inline static uint32_t current_frame_index_{0};
    };
}
