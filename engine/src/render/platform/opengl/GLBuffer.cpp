//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLBuffer.hpp"

namespace engine {
    // --- OpenGL VertexBuffer ---
    GLVertexBuffer::GLVertexBuffer(const void* data, const size_t size, const bool dynamic) {
        glGenBuffers(1, &id_);
        glBindBuffer(GL_ARRAY_BUFFER, id_);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    GLVertexBuffer::~GLVertexBuffer() {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
    }

    GLVertexBuffer::GLVertexBuffer(GLVertexBuffer&& other) noexcept
        : id_(std::exchange(other.id_, 0)) {}

    GLVertexBuffer& GLVertexBuffer::operator=(GLVertexBuffer&& other) noexcept {
        if (this != &other) {
            if (id_ != 0)
                glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    void GLVertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, id_);
    }

    void GLVertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void GLVertexBuffer::set_data(const void* data, const size_t size) {
        glBindBuffer(GL_ARRAY_BUFFER, id_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
    }

    // --- OpenGL IndexBuffer ---
    GLIndexBuffer::GLIndexBuffer(const uint32_t* indices, const uint32_t count)
        : count_(count) {
        glGenBuffers(1, &id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(uint32_t)), indices, GL_STATIC_DRAW);
    }

    GLIndexBuffer::~GLIndexBuffer() {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
    }

    GLIndexBuffer::GLIndexBuffer(GLIndexBuffer&& other) noexcept
        : id_(std::exchange(other.id_, 0)), count_(std::exchange(other.count_, 0)) {}

    GLIndexBuffer& GLIndexBuffer::operator=(GLIndexBuffer&& other) noexcept {
        if (this != &other) {
            if (id_ != 0)
                glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
            count_ = std::exchange(other.count_, 0);
        }
        return *this;
    }

    void GLIndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
    }

    void GLIndexBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    GLStorageBuffer::GLStorageBuffer(size_t size, uint32_t binding, bool dynamic)
        : binding_(binding), size_(size) {
        glGenBuffers(1, &id_);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id_);
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(size), nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_, id_);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    GLStorageBuffer::GLStorageBuffer(const void* data, size_t size, uint32_t binding, bool dynamic)
        : binding_(binding), size_(size) {
        glGenBuffers(1, &id_);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id_);
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(size), data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_, id_);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    GLStorageBuffer::~GLStorageBuffer() {
        if (id_) {
            glDeleteBuffers(1, &id_);
        }
    }

    GLStorageBuffer::GLStorageBuffer(GLStorageBuffer&& other) noexcept
        : id_(other.id_), binding_(other.binding_), size_(other.size_) {
        other.id_ = 0;
        other.size_ = 0;
    }

    GLStorageBuffer& GLStorageBuffer::operator=(GLStorageBuffer&& other) noexcept {
        if (this != &other) {
            if (id_)
                glDeleteBuffers(1, &id_);
            id_ = other.id_;
            binding_ = other.binding_;
            size_ = other.size_;
            other.id_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    void GLStorageBuffer::bind(uint32_t binding) const {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id_);
    }

    void GLStorageBuffer::unbind() const {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void GLStorageBuffer::set_data(const void* data, size_t size, size_t offset) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id_);
        if (offset + size > size_) {
            size_ = (offset + size) * 2;
            glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(size_), nullptr, GL_DYNAMIC_DRAW);
        }
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    }

    void GLStorageBuffer::get_data(void* out_data, size_t size, size_t offset) const {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id_);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), out_data);
    }
}
