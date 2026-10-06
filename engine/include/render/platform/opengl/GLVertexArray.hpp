//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/platform/opengl/GLBuffer.hpp"
#include "render/rhi/BufferLayout.hpp"

namespace engine {
    class GLVertexArray {
    public:
        GLVertexArray();
        ~GLVertexArray();

        GLVertexArray(const GLVertexArray&) = delete;
        GLVertexArray& operator=(const GLVertexArray&) = delete;

        GLVertexArray(GLVertexArray&& other) noexcept;
        GLVertexArray& operator=(GLVertexArray&& other) noexcept;

        void bind() const;
        void unbind() const;

        void add_vertex_buffer(const GLVertexBuffer& vertex_buffer, const BufferLayout& layout);
        void set_index_buffer(const GLIndexBuffer& index_buffer);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }

    private:
        uint32_t renderer_id_ = 0;
        uint32_t attribute_index_ = 0;
        uint32_t binding_index_ = 0;
    };
}
