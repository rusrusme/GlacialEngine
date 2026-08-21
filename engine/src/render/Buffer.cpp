//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/Buffer.hpp"

namespace engine {
    // --- VertexBuffer ---
    VertexBuffer::VertexBuffer(const void* data, const size_t size, const bool dynamic) {
        glGenBuffers(1, &id_);
        glBindBuffer(GL_ARRAY_BUFFER, id_);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    VertexBuffer::~VertexBuffer() {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
        : id_(std::exchange(other.id_, 0)) {}

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
        if (this != &other) {
            if (id_ != 0)
                glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    void VertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, id_);
    }

    void VertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::set_data(const void* data, const size_t size) {
        glBindBuffer(GL_ARRAY_BUFFER, id_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
    }

    // --- IndexBuffer ---
    IndexBuffer::IndexBuffer(const uint32_t* indices, const uint32_t count)
        : count_(count) {
        glGenBuffers(1, &id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(uint32_t)), indices, GL_STATIC_DRAW);
    }

    IndexBuffer::~IndexBuffer() {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : id_(std::exchange(other.id_, 0)), count_(std::exchange(other.count_, 0)) {}

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
        if (this != &other) {
            if (id_ != 0)
                glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
            count_ = std::exchange(other.count_, 0);
        }
        return *this;
    }

    void IndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
    }

    void IndexBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
