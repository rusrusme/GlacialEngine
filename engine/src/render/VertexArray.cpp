//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/VertexArray.hpp"

#include <glad/glad.h>
#include <utility>

namespace engine {
    static GLenum shader_data_type_to_opengl_base_type(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:
            case ShaderDataType::Float2:
            case ShaderDataType::Float3:
            case ShaderDataType::Float4:
            case ShaderDataType::Mat3:
            case ShaderDataType::Mat4: return GL_FLOAT;
            case ShaderDataType::Int:
            case ShaderDataType::Int2:
            case ShaderDataType::Int3:
            case ShaderDataType::Int4: return GL_INT;
            case ShaderDataType::Bool: return GL_BOOL;
            default: return 0;
        }
    }

    VertexArray::VertexArray() {
        glGenVertexArrays(1, &id_);
    }

    VertexArray::~VertexArray() {
        if (id_ != 0) {
            glDeleteVertexArrays(1, &id_);
        }
    }

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : id_(std::exchange(other.id_, 0)),
          attribute_index_(std::exchange(other.attribute_index_, 0)),
          vertex_buffers_(std::move(other.vertex_buffers_)),
          index_buffer_(std::move(other.index_buffer_)) {}

    VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
        if (this != &other) {
            if (id_ != 0)
                glDeleteVertexArrays(1, &id_);
            id_ = std::exchange(other.id_, 0);
            attribute_index_ = std::exchange(other.attribute_index_, 0);
            vertex_buffers_ = std::move(other.vertex_buffers_);
            index_buffer_ = std::move(other.index_buffer_);
        }
        return *this;
    }

    void VertexArray::bind() const {
        glBindVertexArray(id_);
    }

    void VertexArray::unbind() const {
        glBindVertexArray(0);
    }

    void VertexArray::add_vertex_buffer(std::shared_ptr<VertexBuffer> vertex_buffer, const BufferLayout& layout) {
        glBindVertexArray(id_);
        vertex_buffer->bind();

        for (const auto& element : layout) {
            glEnableVertexAttribArray(attribute_index_);
            glVertexAttribPointer(
                attribute_index_,
                static_cast<GLint>(element.get_component_count()),
                shader_data_type_to_opengl_base_type(element.type),
                element.normalized ? GL_TRUE : GL_FALSE,
                static_cast<GLsizei>(layout.get_stride()),
                reinterpret_cast<const void*>(element.offset)
            );
            attribute_index_++;
        }

        vertex_buffers_.push_back(std::move(vertex_buffer));
    }

    void VertexArray::set_index_buffer(std::shared_ptr<IndexBuffer> index_buffer) {
        glBindVertexArray(id_);
        index_buffer->bind();
        index_buffer_ = std::move(index_buffer);
    }
}
