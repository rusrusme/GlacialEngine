//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLCommandBuffer.hpp"
#include "render/platform/opengl/GLDescriptorSet.hpp"
#include "render/platform/opengl/GLFramebuffer.hpp"
#include "render/platform/opengl/GLPipelineState.hpp"
#include "render/rhi/Buffer.hpp"

#include <cassert>
#include <glad/glad.h>

namespace engine {
    namespace {
        constexpr GLenum convert_index_type(IndexType type, size_t& out_size) {
            switch (type) {
                case IndexType::UInt8 : out_size = 1;
                    return GL_UNSIGNED_BYTE;
                case IndexType::UInt16 : out_size = 2;
                    return GL_UNSIGNED_SHORT;
                case IndexType::UInt32 : out_size = 4;
                    return GL_UNSIGNED_INT;
                default : out_size = 4;
                    return GL_UNSIGNED_INT;
            }
        }
    }

    void GLCommandBuffer::begin() noexcept {
        assert(!is_recording_ && "CommandBuffer wird bereits aufgezeichnet!");
        is_recording_ = true;
    }

    void GLCommandBuffer::end() noexcept {
        assert(is_recording_ && "CommandBuffer war nicht im Recording-Zustand!");
        is_recording_ = false;
    }

    void GLCommandBuffer::reset() noexcept {
        is_recording_ = false;
    }

    void GLCommandBuffer::begin_render_pass(const GLFramebuffer& framebuffer, const float clear_color[4]) {
        RenderPassBeginInfo begin_info{};
        begin_info.framebuffer = &framebuffer;
        if (clear_color) {
            begin_info.clear_color = {clear_color[0], clear_color[1], clear_color[2], clear_color[3]};
        }
        begin_info.clear_color_target = true;
        begin_info.clear_depth_target = true;

        begin_render_pass(begin_info);
    }

    void GLCommandBuffer::begin_render_pass(GLFramebuffer& framebuffer, const float clear_color[4]) {
        begin_render_pass(static_cast<const GLFramebuffer&>(framebuffer), clear_color);
    }

    void GLCommandBuffer::begin_render_pass(const RenderPassBeginInfo& begin_info) {
        assert(begin_info.framebuffer && "Framebuffer darf im RenderPass nicht null sein!");

        const uint32_t fbo_id = begin_info.framebuffer->renderer_id();
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        const auto& spec = begin_info.framebuffer->specification();
        glViewport(0, 0, static_cast<GLsizei>(spec.width), static_cast<GLsizei>(spec.height));

        if (fbo_id == 0) {
            if (begin_info.clear_color_target) {
                glClearColor(begin_info.clear_color.r, begin_info.clear_color.g, begin_info.clear_color.b, begin_info.clear_color.a);
            }
            GLbitfield clear_mask = 0;
            if (begin_info.clear_color_target) clear_mask |= GL_COLOR_BUFFER_BIT;
            if (begin_info.clear_depth_target) {
                glClearDepth(begin_info.clear_depth);
                clear_mask |= GL_DEPTH_BUFFER_BIT;
            }
            if (begin_info.clear_stencil_target) {
                glClearStencil(begin_info.clear_stencil);
                clear_mask |= GL_STENCIL_BUFFER_BIT;
            }
            if (clear_mask != 0) {
                glClear(clear_mask);
            }
        }
        else {
            if (begin_info.clear_color_target) {
                const float color[4] = {
                    begin_info.clear_color.r,
                    begin_info.clear_color.g,
                    begin_info.clear_color.b,
                    begin_info.clear_color.a
                };
                for (uint32_t i = 0; i < begin_info.framebuffer->color_attachment_count(); ++i) {
                    glClearNamedFramebufferfv(fbo_id, GL_COLOR, i, color);
                }
            }

            if (begin_info.clear_depth_target && begin_info.clear_stencil_target) {
                glClearNamedFramebufferfi(fbo_id, GL_DEPTH_STENCIL, 0, begin_info.clear_depth, static_cast<GLint>(begin_info.clear_stencil));
            }
            else if (begin_info.clear_depth_target) {
                const float depth = begin_info.clear_depth;
                glClearNamedFramebufferfv(fbo_id, GL_DEPTH, 0, &depth);
            }
            else if (begin_info.clear_stencil_target) {
                const auto stencil = static_cast<GLint>(begin_info.clear_stencil);
                glClearNamedFramebufferiv(fbo_id, GL_STENCIL, 0, &stencil);
            }
        }
    }

    void GLCommandBuffer::end_render_pass() {
        // Unbind / Framebuffer Sync
    }

    void GLCommandBuffer::bind_pipeline(const GLPipelineState& pipeline) {
        pipeline.bind();
    }

    void GLCommandBuffer::bind_vertex_buffer(uint32_t binding_slot, const GLVertexBuffer& buffer, size_t offset, uint32_t stride) {
        glBindVertexBuffer(binding_slot, buffer.renderer_id(), static_cast<GLintptr>(offset), static_cast<GLsizei>(stride));
    }

    void GLCommandBuffer::bind_index_buffer(const GLIndexBuffer& buffer, IndexType index_type) {
        current_index_type_ = index_type;
        gl_index_type_ = convert_index_type(index_type, index_element_size_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.renderer_id());
    }

    void GLCommandBuffer::bind_descriptor_set(DescriptorSetIndex set_index, const GLDescriptorSet& descriptor_set) {
        descriptor_set.bind(set_index);
    }

    void GLCommandBuffer::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void GLCommandBuffer::set_scissor(int32_t x, int32_t y, uint32_t width, uint32_t height) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void GLCommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
        glDrawArraysInstancedBaseInstance(
            GL_TRIANGLES,
            static_cast<GLint>(first_vertex),
            static_cast<GLsizei>(vertex_count),
            static_cast<GLsizei>(instance_count),
            static_cast<GLuint>(first_instance)
        );
    }

    void GLCommandBuffer::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) {
        const void* index_ptr = reinterpret_cast<const void*>(first_index * index_element_size_);
        glDrawElementsInstancedBaseVertexBaseInstance(
            GL_TRIANGLES,
            static_cast<GLsizei>(index_count),
            gl_index_type_,
            index_ptr,
            static_cast<GLsizei>(instance_count),
            static_cast<GLint>(vertex_offset),
            static_cast<GLuint>(first_instance)
        );
    }

    void GLCommandBuffer::draw_indexed_indirect(const GLIndirectBuffer& indirect_buffer, size_t offset, uint32_t draw_count, uint32_t stride) {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer.renderer_id());
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            gl_index_type_,
            reinterpret_cast<const void*>(offset),
            static_cast<GLsizei>(draw_count),
            static_cast<GLsizei>(stride)
        );
    }
} // namespace engine
