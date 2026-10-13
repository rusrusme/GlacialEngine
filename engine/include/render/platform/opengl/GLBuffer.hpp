//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#include <cstdint>

namespace engine {
    class GLVertexBuffer { // VBO
    public:
        explicit GLVertexBuffer(size_t size, const void* data = nullptr,
                                BufferUsageFlags usage = BufferUsageFlags::Vertex,
                                MemoryUsage memory = MemoryUsage::DeviceLocal);
        ~GLVertexBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }
        [[nodiscard]] BufferUsageFlags usage() const { return usage_; }
        [[nodiscard]] MemoryUsage memory_usage() const { return memory_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
        BufferUsageFlags usage_ = BufferUsageFlags::Vertex;
        MemoryUsage memory_ = MemoryUsage::DeviceLocal;
    };

    class GLIndexBuffer { // IBO
    public:
        explicit GLIndexBuffer(size_t size, const uint32_t* indices = nullptr,
                               BufferUsageFlags usage = BufferUsageFlags::Index,
                               MemoryUsage memory = MemoryUsage::DeviceLocal);
        ~GLIndexBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }
        [[nodiscard]] BufferUsageFlags usage() const { return usage_; }
        [[nodiscard]] MemoryUsage memory_usage() const { return memory_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
        BufferUsageFlags usage_ = BufferUsageFlags::Index;
        MemoryUsage memory_ = MemoryUsage::DeviceLocal;
    };

    class GLUniformBuffer { // UBO
    public:
        // `binding` is retained for source compatibility only: buffers no longer
        // grab a global binding point at construction. Bindings are declared by a
        // descriptor set layout and applied by GLDescriptorSet::bind().
        explicit GLUniformBuffer(size_t size, uint32_t binding = 0,
                                 BufferUsageFlags usage = BufferUsageFlags::Uniform,
                                 MemoryUsage memory = MemoryUsage::DeviceLocal);
        ~GLUniformBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }
        [[nodiscard]] BufferUsageFlags usage() const { return usage_; }
        [[nodiscard]] MemoryUsage memory_usage() const { return memory_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
        BufferUsageFlags usage_ = BufferUsageFlags::Uniform;
        MemoryUsage memory_ = MemoryUsage::DeviceLocal;
    };

    class GLShaderStorageBuffer { // SSBO
    public:
        // `binding` is retained for source compatibility only; see GLUniformBuffer.
        explicit GLShaderStorageBuffer(size_t size, uint32_t binding = 0,
                                       BufferUsageFlags usage = BufferUsageFlags::Storage,
                                       MemoryUsage memory = MemoryUsage::DeviceLocal);
        ~GLShaderStorageBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }
        [[nodiscard]] BufferUsageFlags usage() const { return usage_; }
        [[nodiscard]] MemoryUsage memory_usage() const { return memory_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
        BufferUsageFlags usage_ = BufferUsageFlags::Storage;
        MemoryUsage memory_ = MemoryUsage::DeviceLocal;
    };

    class GLIndirectBuffer { // ICB
    public:
        explicit GLIndirectBuffer(size_t size,
                                  BufferUsageFlags usage = BufferUsageFlags::Indirect,
                                  MemoryUsage memory = MemoryUsage::DeviceLocal);
        ~GLIndirectBuffer();

        void set_data(const void* data, size_t size, size_t offset = 0);

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }
        [[nodiscard]] size_t size() const { return size_; }
        [[nodiscard]] BufferUsageFlags usage() const { return usage_; }
        [[nodiscard]] MemoryUsage memory_usage() const { return memory_; }

    private:
        uint32_t renderer_id_ = 0;
        size_t size_ = 0;
        BufferUsageFlags usage_ = BufferUsageFlags::Indirect;
        MemoryUsage memory_ = MemoryUsage::DeviceLocal;
    };
}
