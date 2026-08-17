//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/UniformBuffer.hpp"

#include <utility>
#include <glad/glad.h>

namespace engine {
    UniformBuffer::UniformBuffer(const uint32_t size, const uint32_t binding) {
        glGenBuffers(1, &id_);
        glBindBuffer(GL_UNIFORM_BUFFER, id_);
        glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, id_);
    }

    UniformBuffer::~UniformBuffer() {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
        : id_(std::exchange(other.id_, 0)) {}

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
        if (this != &other) {
            if (id_ != 0)
                glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    void UniformBuffer::set_data(const void* data, const uint32_t size, const uint32_t offset) {
        glBindBuffer(GL_UNIFORM_BUFFER, id_);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    }
}
