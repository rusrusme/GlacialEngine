//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include "GLFW/glfw3.h"

namespace engine {
    struct SwapchainSpec {
        uint32_t width{1920};
        uint32_t height{1080};
        bool vsync{true};
        uint32_t max_frames_in_flight{2};
        void* window_handle{nullptr};
    };

    class GLSwapchain {
    public:
        GLSwapchain() = default;
        explicit GLSwapchain(const SwapchainSpec& spec);
        ~GLSwapchain() = default;

        GLSwapchain(const GLSwapchain&) = delete;
        GLSwapchain& operator=(const GLSwapchain&) = delete;

        GLSwapchain(GLSwapchain&& other) noexcept;
        GLSwapchain& operator=(GLSwapchain&& other) noexcept;

        void present();
        void resize(uint32_t width, uint32_t height);
        void set_vsync(bool enabled);

        [[nodiscard]] bool is_vsync() const noexcept { return spec_.vsync; }
        [[nodiscard]] uint32_t width() const noexcept { return spec_.width; }
        [[nodiscard]] uint32_t height() const noexcept { return spec_.height; }
        [[nodiscard]] uint32_t current_frame_index() const noexcept { return current_frame_index_; }
        [[nodiscard]] uint32_t max_frames_in_flight() const noexcept { return spec_.max_frames_in_flight; }
        [[nodiscard]] const SwapchainSpec& specification() const noexcept { return spec_; }

    private:
        SwapchainSpec spec_{};
        GLFWwindow* window_{nullptr};
        uint32_t current_frame_index_{0};
    };
}
