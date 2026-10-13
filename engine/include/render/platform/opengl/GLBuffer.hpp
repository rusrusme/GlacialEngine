//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine {
    class GLVertexBuffer { // VBO
    public:
        explicit GLVertexBuffer(size_t size, const void* data = nullptr);
        ~GLVertexBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
    };

    class GLIndexBuffer { // IBO
    public:
        explicit GLIndexBuffer(size_t size, const uint32_t* indices = nullptr);
        ~GLIndexBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
    };

    class GLUniformBuffer { // UBO
    public:
        explicit GLUniformBuffer(size_t size, uint32_t binding = 0);
        ~GLUniformBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
    };

    class GLShaderStorageBuffer { // SSBO
    public:
        explicit GLShaderStorageBuffer(size_t size, uint32_t binding = 0);
        ~GLShaderStorageBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
    };

    class GLIndirectBuffer { // ICB
    public:
        explicit GLIndirectBuffer(size_t size);
        ~GLIndirectBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
    };
}
