//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLVertexArray.hpp"

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
            case ShaderDataType::None : break;
        }
        return 0;
    }

    GLVertexArray::GLVertexArray() {
        glCreateVertexArrays(1, &renderer_id_);
    }

    GLVertexArray::~GLVertexArray() {
        glDeleteVertexArrays(1, &renderer_id_);
    }

    GLVertexArray::GLVertexArray(GLVertexArray&& other) noexcept
        : renderer_id_(other.renderer_id_), attribute_index_(other.attribute_index_), binding_index_(other.binding_index_) {
        other.renderer_id_ = 0;
    }

    GLVertexArray& GLVertexArray::operator=(GLVertexArray&& other) noexcept {
        if (this != &other) {
            glDeleteVertexArrays(1, &renderer_id_);
            renderer_id_ = other.renderer_id_;
            attribute_index_ = other.attribute_index_;
            binding_index_ = other.binding_index_;
            other.renderer_id_ = 0;
        }
        return *this;
    }

    void GLVertexArray::bind() const {
        glBindVertexArray(renderer_id_);
    }

    void GLVertexArray::unbind() const {
        glBindVertexArray(0);
    }

    void GLVertexArray::add_vertex_buffer(const GLVertexBuffer& vertex_buffer, const BufferLayout& layout, uint32_t divisor) {
        for (const auto& element : layout) {
            GLenum base_type = shader_data_type_to_opengl_base_type(element.type);

            if (element.type == ShaderDataType::Mat3 || element.type == ShaderDataType::Mat4) {
                const uint32_t count = (element.type == ShaderDataType::Mat4) ? 4 : 3;
                for (uint32_t i = 0; i < count; ++i) {
                    glEnableVertexArrayAttrib(renderer_id_, attribute_index_);
                    glVertexArrayAttribFormat(
                        renderer_id_,
                        attribute_index_,
                        count,
                        base_type,
                        element.normalized ? GL_TRUE : GL_FALSE,
                        static_cast<GLuint>(element.offset + i * count * sizeof(float))
                    );
                    glVertexArrayAttribBinding(renderer_id_, attribute_index_, binding_index_);
                    attribute_index_++;
                }
            }
            else {
                glEnableVertexArrayAttrib(renderer_id_, attribute_index_);

                if (element.type == ShaderDataType::Int || element.type == ShaderDataType::Int2 ||
                    element.type == ShaderDataType::Int3 || element.type == ShaderDataType::Int4 ||
                    element.type == ShaderDataType::Bool) {
                    glVertexArrayAttribIFormat(
                        renderer_id_,
                        attribute_index_,
                        static_cast<GLint>(element.component_count()),
                        base_type,
                        static_cast<GLuint>(element.offset)
                    );
                }
                else {
                    glVertexArrayAttribFormat(
                        renderer_id_,
                        attribute_index_,
                        static_cast<GLint>(element.component_count()),
                        base_type,
                        element.normalized ? GL_TRUE : GL_FALSE,
                        static_cast<GLuint>(element.offset)
                    );
                }
                glVertexArrayAttribBinding(renderer_id_, attribute_index_, binding_index_);
                attribute_index_++;
            }
        }
        glVertexArrayVertexBuffer(renderer_id_, binding_index_, vertex_buffer.renderer_id(), 0, static_cast<GLsizei>(layout.stride()));

        if (divisor > 0) {
            glVertexArrayBindingDivisor(renderer_id_, binding_index_, divisor);
        }

        binding_index_++;
    }

    void GLVertexArray::set_index_buffer(const GLIndexBuffer& index_buffer) {
        glVertexArrayElementBuffer(renderer_id_, index_buffer.renderer_id());
    }
}
