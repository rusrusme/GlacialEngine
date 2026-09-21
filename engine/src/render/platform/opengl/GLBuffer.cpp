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
}
