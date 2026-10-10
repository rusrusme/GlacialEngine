//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <glad/glad.h>

namespace engine {
    class GLFence {
    public:
        GLFence() = default;
        ~GLFence();

        GLFence(const GLFence&) = delete;
        GLFence& operator=(const GLFence&) = delete;

        GLFence(GLFence&& other) noexcept;
        GLFence& operator=(GLFence&& other) noexcept;

        void signal();
        [[nodiscard]] bool wait(uint64_t timeout_ns = UINT64_MAX);
        void gpu_wait();
        void reset();

        [[nodiscard]] bool is_signaled() const;
        [[nodiscard]] GLsync native_handle() const noexcept { return sync_obj_; }

    private:
        void release();

        GLsync sync_obj_{nullptr};
    };
}