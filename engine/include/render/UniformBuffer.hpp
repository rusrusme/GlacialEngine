//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine {
    class UniformBuffer {
    public:
        UniformBuffer(uint32_t size, uint32_t binding);
        ~UniformBuffer();

        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        UniformBuffer(UniformBuffer&& other) noexcept;
        UniformBuffer& operator=(UniformBuffer&& other) noexcept;

        void set_data(const void* data, uint32_t size, uint32_t offset = 0);

    private:
        uint32_t id_ = 0;
    };
}
