//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine {
    class GLVertexBuffer { // VBO
    public:
        GLVertexBuffer(const void* data, size_t size);
        ~GLVertexBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);
        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }

    private:
        uint32_t renderer_id_ = 0;
    };

    class GLIndexBuffer { // IBO
    public:
        GLIndexBuffer(const uint32_t* indices, uint32_t count);
        ~GLIndexBuffer();

        [[nodiscard]] uint32_t count() const { return count_; }
        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }

    private:
        uint32_t renderer_id_ = 0;
        uint32_t count_ = 0;
    };

    class GLUniformBuffer { // UBO
    public:
        GLUniformBuffer(size_t size, uint32_t binding);
        ~GLUniformBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);
        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }

    private:
        uint32_t renderer_id_ = 0;
    };

    class GLShaderStorageBuffer { // SSBO
    public:
        GLShaderStorageBuffer(size_t size, uint32_t binding);
        ~GLShaderStorageBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);
        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }

    private:
        uint32_t renderer_id_ = 0;
    };

    class GLIndirectBuffer { // ICB
    public:
        explicit GLIndirectBuffer(size_t size);
        ~GLIndirectBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);
        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }

    private:
        uint32_t renderer_id_ = 0;
    };
}
