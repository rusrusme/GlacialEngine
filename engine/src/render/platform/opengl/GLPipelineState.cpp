//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLPipelineState.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <glad/glad.h>

#include "core/Log.hpp"

namespace engine {
    static constexpr GLenum convert_shader_data_type(ShaderDataType type) {
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
            default : return GL_FLOAT;
        }
    }

    static constexpr GLenum convert_cull_mode(CullMode mode) {
        switch (mode) {
            case CullMode::Front : return GL_FRONT;
            case CullMode::Back : return GL_BACK;
            case CullMode::FrontAndBack : return GL_FRONT_AND_BACK;
            default : return GL_BACK;
        }
    }

    static constexpr GLenum convert_polygon_mode(PolygonMode mode) {
        switch (mode) {
            case PolygonMode::Fill : return GL_FILL;
            case PolygonMode::Wireframe : return GL_LINE;
            case PolygonMode::Point : return GL_POINT;
            default : return GL_FILL;
        }
    }

    static constexpr GLenum convert_compare_op(CompareOp op) {
        switch (op) {
            case CompareOp::Never : return GL_NEVER;
            case CompareOp::Less : return GL_LESS;
            case CompareOp::Equal : return GL_EQUAL;
            case CompareOp::LessOrEqual : return GL_LEQUAL;
            case CompareOp::Greater : return GL_GREATER;
            case CompareOp::NotEqual : return GL_NOTEQUAL;
            case CompareOp::GreaterOrEqual : return GL_GEQUAL;
            case CompareOp::Always : return GL_ALWAYS;
            default : return GL_LESS;
        }
    }

    GLPipelineState::GLPipelineState(PipelineStateDesc desc)
        : desc_(std::move(desc)) {
        if (!desc_.shader) {
            CORE_ERROR("[GLPipelineState] Shader is invalid");
            return;
        }
        setup_vertex_array();
    }

    GLPipelineState::~GLPipelineState() {
        release();
    }

    GLPipelineState::GLPipelineState(GLPipelineState&& other) noexcept
        : desc_(std::move(other.desc_)), vao_id_(other.vao_id_) {
        other.vao_id_ = 0;
    }

    GLPipelineState& GLPipelineState::operator=(GLPipelineState&& other) noexcept {
        if (this != &other) {
            release();
            desc_ = std::move(other.desc_);
            vao_id_ = other.vao_id_;
            other.vao_id_ = 0;
        }
        return *this;
    }

    void GLPipelineState::bind() const {
        if (!desc_.shader) {
            CORE_ERROR("[GLPipelineState] Shader is invalid");
            return;
        }
        desc_.shader->bind();

        glBindVertexArray(vao_id_);

        if (desc_.cull_mode == CullMode::None) {
            glDisable(GL_CULL_FACE);
        }
        else {
            glEnable(GL_CULL_FACE);
            glCullFace(convert_cull_mode(desc_.cull_mode));
        }

        glPolygonMode(GL_FRONT_AND_BACK, convert_polygon_mode(desc_.polygon_mode));

        if (desc_.depth_test) {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(desc_.depth_write ? GL_TRUE : GL_FALSE);
            glDepthFunc(convert_compare_op(desc_.depth_compare));
        }
        else {
            glDisable(GL_DEPTH_TEST);
        }

        if (desc_.blend_enable) {
            glEnable(GL_BLEND);
            switch (desc_.blend_mode) {
                case BlendMode::Additive:
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    break;
                case BlendMode::Alpha:
                default:
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    break;
            }
        }
        else {
            glDisable(GL_BLEND);
        }

        if (desc_.polygon_offset_enable) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(desc_.polygon_offset_factor, desc_.polygon_offset_units);
        } else {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    void GLPipelineState::setup_vertex_array() {
        glCreateVertexArrays(1, &vao_id_);

        const auto& layout = desc_.vertex_layout;
        uint32_t next_location = 0;

        for (const auto& element : layout) {
            // Explicit locations are honoured as declared; otherwise locations are
            // handed out in declaration order, starting past whatever the previous
            // elements consumed.
            const uint32_t location = (element.location == kInvalidAttributeLocation) ? next_location : element.location;
            const uint32_t binding = element.binding_index;

            // A matrix element occupies one attribute location per column; every
            // other element occupies exactly one.
            const uint32_t column_count = element.location_count();

            for (uint32_t column = 0; column < column_count; ++column) {
                const uint32_t attribute_location = location + column;
                const uint32_t components = element.is_matrix() ? column_count : element.component_count();
                const size_t byte_offset = element.offset +
                                           (element.is_matrix() ? sizeof(float) * column_count * column : 0);

                glEnableVertexArrayAttrib(vao_id_, attribute_location);

                if (element.is_integer()) {
                    glVertexArrayAttribIFormat(
                        vao_id_,
                        attribute_location,
                        static_cast<GLint>(components),
                        convert_shader_data_type(element.type),
                        static_cast<GLuint>(byte_offset)
                    );
                }
                else {
                    glVertexArrayAttribFormat(
                        vao_id_,
                        attribute_location,
                        static_cast<GLint>(components),
                        convert_shader_data_type(element.type),
                        element.normalized ? GL_TRUE : GL_FALSE,
                        static_cast<GLuint>(byte_offset)
                    );
                }

                glVertexArrayAttribBinding(vao_id_, attribute_location, binding);
            }

            next_location = std::max(next_location, location + column_count);

            // The divisor applies to the whole binding point, so elements sharing a
            // binding_index must agree on per_instance.
            glVertexArrayBindingDivisor(vao_id_, binding, element.divisor());
        }
    }

    void GLPipelineState::release() {
        if (vao_id_ != 0) {
            glDeleteVertexArrays(1, &vao_id_);
            vao_id_ = 0;
        }
    }
} // engine
