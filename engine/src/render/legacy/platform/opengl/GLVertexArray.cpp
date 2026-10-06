//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/platform/opengl/GLVertexArray.hpp"
#include "render/legacy/Buffer.hpp"

namespace engine {
    static GLenum shader_data_type_to_opengl_base_type(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float :
            case ShaderDataType::Float2 :
            case ShaderDataType::Float3 :
            case ShaderDataType::Float4 :
            case ShaderDataType::Mat3 :
            case ShaderDataType::Mat4 : return GL_FLOAT;
            case ShaderDataType::Int :
            case ShaderDataType::Int2 :
            case ShaderDataType::Int3 :
            case ShaderDataType::Int4 : return GL_INT;
            case ShaderDataType::Bool : return GL_BOOL;
            default : return 0;
        }
    }

    GLVertexArray::GLVertexArray() {
        glGenVertexArrays(1, &id_);
    }

    GLVertexArray::~GLVertexArray() {
        if (id_ != 0) {
            glDeleteVertexArrays(1, &id_);
        }
    }

    GLVertexArray::GLVertexArray(GLVertexArray&& other) noexcept
        : id_(std::exchange(other.id_, 0)),
          attribute_index_(std::exchange(other.attribute_index_, 0)),
          vertex_buffers_(std::move(other.vertex_buffers_)),
          index_buffer_(std::move(other.index_buffer_)) {}

    GLVertexArray& GLVertexArray::operator=(GLVertexArray&& other) noexcept {
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

    void GLVertexArray::bind() const {
        glBindVertexArray(id_);
    }

    void GLVertexArray::unbind() const {
        glBindVertexArray(0);
    }

    void GLVertexArray::add_vertex_buffer(std::shared_ptr<GLVertexBuffer> vertex_buffer, const BufferLayout& layout) {
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

    void GLVertexArray::set_index_buffer(std::shared_ptr<GLIndexBuffer> index_buffer) {
        glBindVertexArray(id_);
        index_buffer->bind();
        index_buffer_ = std::move(index_buffer);
    }

    void GLVertexArray::add_instanced_buffer(const std::shared_ptr<GLVertexBuffer>& vertex_buffer, const BufferLayout& layout, uint32_t divisor) {
        glBindVertexArray(id_);
        vertex_buffer->bind();

        for (const auto& element : layout) {
            if (element.type == ShaderDataType::Mat4) {
                for (uint32_t i = 0; i < 4; ++i) {
                    glEnableVertexAttribArray(attribute_index_);
                    glVertexAttribPointer(
                        attribute_index_,
                        4,
                        GL_FLOAT,
                        element.normalized ? GL_TRUE : GL_FALSE,
                        static_cast<GLsizei>(layout.get_stride()),
                        reinterpret_cast<const void*>(element.offset + sizeof(glm::vec4) * i)
                    );
                    glVertexAttribDivisor(attribute_index_, divisor);
                    attribute_index_++;
                }
            }
            else {
                glEnableVertexAttribArray(attribute_index_);
                glVertexAttribPointer(
                    attribute_index_,
                    static_cast<GLint>(element.get_component_count()),
                    shader_data_type_to_opengl_base_type(element.type),
                    element.normalized ? GL_TRUE : GL_FALSE,
                    static_cast<GLsizei>(layout.get_stride()),
                    reinterpret_cast<const void*>(element.offset)
                );
                glVertexAttribDivisor(attribute_index_, divisor);
                attribute_index_++;
            }
        }
        vertex_buffers_.push_back(vertex_buffer);
    }
}
