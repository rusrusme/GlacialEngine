//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLCommandBuffer.hpp"

#include "core/Log.hpp"
#include "render/platform/opengl/GLDescriptorSet.hpp"
#include "render/platform/opengl/GLFramebuffer.hpp"
#include "render/platform/opengl/GLPipelineState.hpp"
#include "render/rhi/Buffer.hpp"

#include <cassert>
#include <cstdint>
#include <glad/glad.h>

namespace engine {
    namespace {
        constexpr GLenum convert_index_type(const IndexType type, size_t& out_size) {
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
        assert(!is_recording_ && "CommandBuffer is already recording");
        is_recording_ = true;
    }

    void GLCommandBuffer::end() noexcept {
        assert(is_recording_ && "CommandBuffer was not in recording state");
        is_recording_ = false;
    }

    void GLCommandBuffer::reset() noexcept {
        // OpenGL has no command storage to rewind; pipeline/vertex state is not
        // restored here either, so a reset buffer keeps whatever VAO was last bound.
        is_recording_ = false;
    }

    void GLCommandBuffer::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) {
        glDispatchCompute(
            static_cast<GLuint>(group_x),
            static_cast<GLuint>(group_y),
            static_cast<GLuint>(group_z)
        );
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

    void GLCommandBuffer::begin_render_pass(const GLFramebuffer& framebuffer, const RenderPassDesc& pass_desc, const ClearValue& clear_value) {
        RenderPassBeginInfo begin_info{};
        begin_info.framebuffer = &framebuffer;
        begin_info.clear_color = {clear_value.color[0], clear_value.color[1], clear_value.color[2], clear_value.color[3]};
        begin_info.clear_depth = clear_value.depth;
        begin_info.clear_stencil = clear_value.stencil;

        for (const RenderPassAttachmentDesc& attachment : pass_desc.attachments) {
            if (attachment.load_op != AttachmentLoadOp::Clear) {
                continue;
            }
            if (attachment.type == AttachmentType::Color) {
                begin_info.clear_color_target = true;
            }
            else if (attachment.type == AttachmentType::Depth) {
                begin_info.clear_depth_target = true;
            }
            else { // DepthStencil
                begin_info.clear_depth_target = true;
                begin_info.clear_stencil_target = true;
            }
        }

        begin_render_pass(begin_info);
    }

    void GLCommandBuffer::begin_render_pass(const RenderPassBeginInfo& begin_info) {
        assert(begin_info.framebuffer && "Framebuffer cannot be nullptr in render pass");

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
                glClearStencil(static_cast<GLint>(begin_info.clear_stencil));
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
                    glClearNamedFramebufferfv(fbo_id, GL_COLOR, static_cast<GLint>(i), color);
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

        bound_pipeline_ = &pipeline;
        bound_vao_ = pipeline.vao_id();

        // Re-apply everything recorded so far onto the pipeline's VAO. Without this
        // a binding issued before bind_pipeline would either be lost or land in the
        // previously bound VAO.
        apply_vertex_bindings();
        attach_index_buffer();
    }

    void GLCommandBuffer::bind_vertex_buffer(uint32_t binding_slot, const GLVertexBuffer& buffer, size_t offset, uint32_t stride) {
        if (binding_slot >= kMaxVertexBindings) {
            CORE_ERROR("[GLCommandBuffer] Vertex binding slot {} exceeds the maximum supported slot {}",
                       binding_slot, kMaxVertexBindings - 1);
            return;
        }

        // A stride of zero is not meaningful for an interleaved vertex layout, so
        // fall back to the stride declared by the bound pipeline's vertex layout.
        if (stride == 0 && bound_pipeline_) {
            stride = bound_pipeline_->description().vertex_layout.stride();
        }

        vertex_bindings_[binding_slot] = VertexBindingState{buffer.renderer_id(), offset, stride, true};

        if (bound_vao_ != 0) {
            glVertexArrayVertexBuffer(bound_vao_, binding_slot, buffer.renderer_id(),
                                      static_cast<GLintptr>(offset), static_cast<GLsizei>(stride));
        }
    }

    void GLCommandBuffer::bind_index_buffer(const GLIndexBuffer& buffer, IndexType index_type) {
        current_index_type_ = index_type;
        pending_index_buffer_ = buffer.renderer_id();
        has_index_buffer_ = true;

        attach_index_buffer();
    }

    void GLCommandBuffer::apply_vertex_bindings() const {
        if (bound_vao_ == 0) {
            return;
        }

        for (uint32_t slot = 0; slot < kMaxVertexBindings; ++slot) {
            const VertexBindingState& binding = vertex_bindings_[slot];
            if (!binding.valid) {
                continue;
            }
            glVertexArrayVertexBuffer(bound_vao_, slot, binding.buffer_id,
                                      static_cast<GLintptr>(binding.offset), static_cast<GLsizei>(binding.stride));
        }
    }

    void GLCommandBuffer::attach_index_buffer() const {
        if (bound_vao_ != 0 && pending_index_buffer_ != 0) {
            glVertexArrayElementBuffer(bound_vao_, pending_index_buffer_);
        }
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

    void GLCommandBuffer::memory_barrier(MemoryBarrierFlags flags) {
        GLbitfield barriers = 0;

        if ((flags & MemoryBarrierFlags::ShaderStorage) != MemoryBarrierFlags::None) {
            barriers |= GL_SHADER_STORAGE_BARRIER_BIT;
        }
        if ((flags & MemoryBarrierFlags::UniformBuffer) != MemoryBarrierFlags::None) {
            barriers |= GL_UNIFORM_BARRIER_BIT;
        }
        if ((flags & MemoryBarrierFlags::TextureFetch) != MemoryBarrierFlags::None) {
            barriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT;
        }
        if ((flags & MemoryBarrierFlags::CommandData) != MemoryBarrierFlags::None) {
            barriers |= GL_COMMAND_BARRIER_BIT;
        }
        if ((flags & MemoryBarrierFlags::FramebufferAttachment) != MemoryBarrierFlags::None) {
            barriers |= GL_FRAMEBUFFER_BARRIER_BIT;
        }

        if (barriers != 0) {
            glMemoryBarrier(barriers);
        }
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
        if (!has_index_buffer_ || pending_index_buffer_ == 0) {
            CORE_ERROR("[GLCommandBuffer] draw_indexed called without a bound index buffer");
            return;
        }

        // Derived from the currently bound index type instead of a cached size, so
        // the element offset can never lag behind an index buffer swap.
        size_t element_size = 0;
        const GLenum gl_index_type = convert_index_type(current_index_type_, element_size);

        const auto index_ptr = reinterpret_cast<const void*>(static_cast<uintptr_t>(first_index) * element_size);
        glDrawElementsInstancedBaseVertexBaseInstance(
            GL_TRIANGLES,
            static_cast<GLsizei>(index_count),
            gl_index_type,
            index_ptr,
            static_cast<GLsizei>(instance_count),
            static_cast<GLint>(vertex_offset),
            static_cast<GLuint>(first_instance)
        );
    }

    void GLCommandBuffer::draw_indexed_indirect(const GLIndirectBuffer& indirect_buffer, size_t offset, uint32_t draw_count, uint32_t stride) {
        if (!has_index_buffer_ || pending_index_buffer_ == 0) {
            CORE_ERROR("[GLCommandBuffer] draw_indexed_indirect called without a bound index buffer");
            return;
        }

        size_t element_size = 0;
        const GLenum gl_index_type = convert_index_type(current_index_type_, element_size);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer.renderer_id());
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            gl_index_type,
            reinterpret_cast<const void*>(offset),
            static_cast<GLsizei>(draw_count),
            static_cast<GLsizei>(stride)
        );
    }
} // namespace engine
