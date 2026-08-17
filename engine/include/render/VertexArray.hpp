//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <vector>

#include "Buffer.hpp"
#include "BufferLayout.hpp"

namespace engine {
    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        VertexArray(VertexArray&& other) noexcept;
        VertexArray& operator=(VertexArray&& other) noexcept;

        void bind() const;
        void unbind() const;

        void add_vertex_buffer(std::shared_ptr<VertexBuffer> vertex_buffer, const BufferLayout& layout);
        void set_index_buffer(std::shared_ptr<IndexBuffer> index_buffer);

        [[nodiscard]] const std::vector<std::shared_ptr<VertexBuffer>>& get_vertex_buffers() const { return vertex_buffers_; }
        [[nodiscard]] const std::shared_ptr<IndexBuffer>& get_index_buffer() const { return index_buffer_; }

    private:
        uint32_t id_ = 0;
        uint32_t attribute_index_ = 0;
        std::vector<std::shared_ptr<VertexBuffer>> vertex_buffers_;
        std::shared_ptr<IndexBuffer> index_buffer_;
    };
};
