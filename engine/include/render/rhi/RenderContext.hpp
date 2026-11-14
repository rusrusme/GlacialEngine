//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <array>
#include <memory>

#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/RHIDevice.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Swapchain.hpp"

namespace engine {
    class Window;

    class RenderContext {
    public:
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        void init(const Window& window);

        void shutdown();

        void begin_frame();

        void end_frame();

        void on_window_resize(uint32_t width, uint32_t height);

        [[nodiscard]] RHIDevice& device() noexcept { return device_; }
        [[nodiscard]] Swapchain& swapchain() noexcept { return *swapchain_; }

        [[nodiscard]] CommandBuffer& active_command_buffer() const { return *command_buffers_[current_frame_index_]; }
        [[nodiscard]] uint32_t current_frame_index() const noexcept { return current_frame_index_; }

    private:
        RHIDevice device_{};
        Shared<Swapchain> swapchain_{nullptr};
        std::array<Unique<CommandBuffer>, MAX_FRAMES_IN_FLIGHT> command_buffers_{};
        uint32_t current_frame_index_{0};
    };
}
