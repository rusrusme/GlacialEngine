//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine {
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

    class GLStorageBuffer {
    public:
        GLStorageBuffer(size_t size, uint32_t binding = 0, bool dynamic = true);
        GLStorageBuffer(const void* data, size_t size, uint32_t binding = 0, bool dynamic = true);
        ~GLStorageBuffer();

        GLStorageBuffer(const GLStorageBuffer&) = delete;
        GLStorageBuffer& operator=(const GLStorageBuffer&) = delete;

        GLStorageBuffer(GLStorageBuffer&& other) noexcept;
        GLStorageBuffer& operator=(GLStorageBuffer&& other) noexcept;

        void bind(uint32_t binding = 0) const;
        void unbind() const;

        void set_data(const void* data, size_t size, size_t offset = 0);
        void get_data(void* out_data, size_t size, size_t offset = 0) const;

        [[nodiscard]] size_t get_size() const { return size_; }

    private:
        uint32_t id_ = 0;
        uint32_t binding_ = 0;
        size_t size_ = 0;
    };
}