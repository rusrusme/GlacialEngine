//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine::legacy {
    using namespace engine;
    class GLVertexBuffer {
    public:
        GLVertexBuffer(const void* data, size_t size, bool dynamic = false);
        ~GLVertexBuffer();

        GLVertexBuffer(const GLVertexBuffer&) = delete;
        GLVertexBuffer& operator=(const GLVertexBuffer&) = delete;

        GLVertexBuffer(GLVertexBuffer&& other) noexcept;
        GLVertexBuffer& operator=(GLVertexBuffer&& other) noexcept;

        void bind() const;
        void unbind() const;

        void set_data(const void* data, size_t size);

    private:
        uint32_t id_ = 0;
    };

    class GLIndexBuffer {
    public:
        GLIndexBuffer(const uint32_t* indices, uint32_t count);
        ~GLIndexBuffer();

        GLIndexBuffer(const GLIndexBuffer&) = delete;
        GLIndexBuffer& operator=(const GLIndexBuffer&) = delete;

        GLIndexBuffer(GLIndexBuffer&& other) noexcept;
        GLIndexBuffer& operator=(GLIndexBuffer&& other) noexcept;

        void bind() const;
        void unbind() const;

        [[nodiscard]] uint32_t get_count() const { return count_; }

    private:
        uint32_t id_ = 0;
        uint32_t count_ = 0;
    };
}