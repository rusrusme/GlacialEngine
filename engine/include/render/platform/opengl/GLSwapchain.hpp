//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

namespace engine {
    class GLFramebuffer;

    class GLSwapchain {
    public:
        explicit GLSwapchain(const SwapchainSpec& spec);
        ~GLSwapchain() = default;

        GLSwapchain(const GLSwapchain&) = delete;
        GLSwapchain& operator=(const GLSwapchain&) = delete;

        GLSwapchain(GLSwapchain&& other) noexcept;
        GLSwapchain& operator=(GLSwapchain&& other) noexcept;

        void present();
        void resize(uint32_t width, uint32_t height);

        void set_vsync(bool enabled);
        [[nodiscard]] bool vsync() const noexcept { return spec_.vsync; }

        [[nodiscard]] GLFramebuffer& backbuffer() const { return *backbuffer_; }

        [[nodiscard]] uint32_t width() const noexcept { return spec_.width; }
        [[nodiscard]] uint32_t height() const noexcept { return spec_.height; }

    private:
        SwapchainSpec spec_{};
        void* window_{nullptr};
        Unique<GLFramebuffer> backbuffer_{nullptr};
    };
}
