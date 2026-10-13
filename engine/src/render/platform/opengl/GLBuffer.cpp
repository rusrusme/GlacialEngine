//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLBuffer.hpp"

#include <glad/glad.h>

namespace engine {
    // GLVertexBuffer (VBO)
    GLVertexBuffer::GLVertexBuffer(size_t size, const void* data)
        : size_(size) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), data, GL_DYNAMIC_STORAGE_BIT);
    }

    GLVertexBuffer::~GLVertexBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLVertexBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    // GLIndexBuffer (IBO)
    GLIndexBuffer::GLIndexBuffer(size_t size, const uint32_t* indices)
        : size_(size) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), indices, GL_DYNAMIC_STORAGE_BIT);
    }

    GLIndexBuffer::~GLIndexBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLIndexBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    // GLUniformBuffer (UBO)
    GLUniformBuffer::GLUniformBuffer(size_t size, uint32_t binding)
        : size_(size) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, renderer_id_);
    }

    GLUniformBuffer::~GLUniformBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLUniformBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    // GLShaderStorageBuffer (SSBO)
    GLShaderStorageBuffer::GLShaderStorageBuffer(size_t size, uint32_t binding)
        : size_(size) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, renderer_id_);
    }

    GLShaderStorageBuffer::~GLShaderStorageBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLShaderStorageBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    // GLIndirectBuffer (ICB)
    GLIndirectBuffer::GLIndirectBuffer(size_t size)
        : size_(size) {
        glCreateBuffers(1, &renderer_id_);
        glNamedBufferStorage(renderer_id_, static_cast<GLsizeiptr>(size_), nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    GLIndirectBuffer::~GLIndirectBuffer() {
        glDeleteBuffers(1, &renderer_id_);
    }

    void GLIndirectBuffer::set_data(const void* data, size_t size, size_t offset) {
        glNamedBufferSubData(renderer_id_, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }
}
