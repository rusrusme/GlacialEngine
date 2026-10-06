//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <vector>

#include "render/legacy/BufferLayout.hpp"

namespace engine {
    class GLIndexBuffer;
    class GLVertexBuffer;

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

        void add_vertex_buffer(std::shared_ptr<GLVertexBuffer> vertex_buffer, const BufferLayout& layout);
        void set_index_buffer(std::shared_ptr<GLIndexBuffer> index_buffer);

        void add_instanced_buffer(const std::shared_ptr<GLVertexBuffer>& vertex_buffer, const BufferLayout& layout, uint32_t divisor = 1);

        [[nodiscard]] const std::vector<std::shared_ptr<GLVertexBuffer>>& get_vertex_buffers() const { return vertex_buffers_; }
        [[nodiscard]] const std::shared_ptr<GLIndexBuffer>& get_index_buffer() const { return index_buffer_; }

    private:
        uint32_t id_ = 0;
        uint32_t attribute_index_ = 0;
        std::vector<std::shared_ptr<GLVertexBuffer>> vertex_buffers_;
        std::shared_ptr<GLIndexBuffer> index_buffer_;
    };
}
