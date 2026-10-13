//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLBuffer.hpp"

#include <glad/glad.h>

namespace engine {
    namespace {
        // glNamedBufferSubData (used by set_data) requires dynamic storage, so it is
        // always requested. Host visible buffers additionally become mappable.
        GLbitfield storage_flags(const MemoryUsage memory) {
            switch (memory) {
                case MemoryUsage::HostVisibleCoherent :
                    return GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT |
                           GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
                case MemoryUsage::HostVisible :
                    return GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
                case MemoryUsage::DeviceLocal :
                default :
                    return GL_DYNAMIC_STORAGE_BIT;
            }
        }
    }

    // GLVertexBuffer (VBO)
    GLVertexBuffer::GLVertexBuffer(size_t size, const void* data, BufferUsageFlags usage, MemoryUsage memory)
        : size_(size), usage_(usage), memory_(memory) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), data, storage_flags(memory_));
    }

    GLVertexBuffer::~GLVertexBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLVertexBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    // GLIndexBuffer (IBO)
    GLIndexBuffer::GLIndexBuffer(size_t size, const uint32_t* indices, BufferUsageFlags usage, MemoryUsage memory)
        : size_(size), usage_(usage), memory_(memory) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), indices, storage_flags(memory_));
    }

    GLIndexBuffer::~GLIndexBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLIndexBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    // GLUniformBuffer (UBO)
    GLUniformBuffer::GLUniformBuffer(size_t size, [[maybe_unused]] uint32_t binding, BufferUsageFlags usage, MemoryUsage memory)
        : size_(size), usage_(usage), memory_(memory) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), nullptr, storage_flags(memory_));
    }

    GLUniformBuffer::~GLUniformBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLUniformBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    // GLShaderStorageBuffer (SSBO)
    GLShaderStorageBuffer::GLShaderStorageBuffer(size_t size, [[maybe_unused]] uint32_t binding, BufferUsageFlags usage, MemoryUsage memory)
        : size_(size), usage_(usage), memory_(memory) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), nullptr, storage_flags(memory_));
    }

    GLShaderStorageBuffer::~GLShaderStorageBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLShaderStorageBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    // GLIndirectBuffer (ICB)
    GLIndirectBuffer::GLIndirectBuffer(size_t size, BufferUsageFlags usage, MemoryUsage memory)
        : size_(size), usage_(usage), memory_(memory) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), nullptr, storage_flags(memory_));
    }

    GLIndirectBuffer::~GLIndirectBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLIndirectBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }
}
