//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine::legacy {
    using namespace engine;
    class GLUniformBuffer {
    public:
        GLUniformBuffer(uint32_t size, uint32_t binding);
        ~GLUniformBuffer();

        GLUniformBuffer(const GLUniformBuffer&) = delete;
        GLUniformBuffer& operator=(const GLUniformBuffer&) = delete;

        GLUniformBuffer(GLUniformBuffer&& other) noexcept;
        GLUniformBuffer& operator=(GLUniformBuffer&& other) noexcept;

        void set_data(const void* data, uint32_t size, uint32_t offset = 0);

    private:
        uint32_t id_ = 0;
    };
}