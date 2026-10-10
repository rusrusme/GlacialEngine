//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLComputePipeline.hpp"

#include <glad/glad.h>

#include "core/Log.hpp"
#include "render/platform/opengl/GLBuffer.hpp"

namespace engine {
    GLComputePipeline::GLComputePipeline(ComputePipelineDesc desc)
        : desc_(std::move(desc)) {
        if (!desc_.shader) {
            CORE_ERROR("[GLComputePipeline] Shader is null");
        }
    }

    void GLComputePipeline::bind() const {
        if (!desc_.shader) {
            CORE_ERROR("[GLComputePipeline] Attempted to bind pipeline without a valid shader");
            return;
        }
        desc_.shader->bind();
    }

    void GLComputePipeline::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const {
        glDispatchCompute(
            static_cast<GLuint>(group_x),
            static_cast<GLuint>(group_y),
            static_cast<GLuint>(group_z)
        );
    }

    void GLComputePipeline::dispatch_indirect(const GLIndirectBuffer& indirect_buffer, size_t offset) const {
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirect_buffer.renderer_id());
        glDispatchComputeIndirect(static_cast<GLintptr>(offset));
    }

    void GLComputePipeline::set_memory_barrier(MemoryBarrierFlags flags) {
        if (flags == MemoryBarrierFlags::All) {
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            return;
        }

        GLbitfield gl_flags = 0;
        const auto mask = static_cast<uint32_t>(flags);

        if (mask & static_cast<uint32_t>(MemoryBarrierFlags::ShaderStorage)) {
            gl_flags |= GL_SHADER_STORAGE_BARRIER_BIT;
        }
        if (mask & static_cast<uint32_t>(MemoryBarrierFlags::UniformBuffer)) {
            gl_flags |= GL_UNIFORM_BARRIER_BIT;
        }
        if (mask & static_cast<uint32_t>(MemoryBarrierFlags::TextureFetch)) {
            gl_flags |= GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
        }
        if (mask & static_cast<uint32_t>(MemoryBarrierFlags::CommandData)) {
            gl_flags |= GL_COMMAND_BARRIER_BIT;
        }
        if (mask & static_cast<uint32_t>(MemoryBarrierFlags::FramebufferAttachment)) {
            gl_flags |= GL_FRAMEBUFFER_BARRIER_BIT;
        }

        if (gl_flags != 0) {
            glMemoryBarrier(gl_flags);
        }
    }
}
