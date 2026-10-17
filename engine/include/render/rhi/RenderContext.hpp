//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <array>
#include <memory>

#include "core/Window.hpp"
#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/RHIDevice.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Swapchain.hpp"

namespace engine {
    class RenderContext {
    public:
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        void init(const Window& window) {
            device_.init();

            SwapchainSpec spec{};
            spec.width = window.width();
            spec.height = window.height();
            spec.vsync = window.vsync();
            spec.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
            spec.window_handle = window.native();

            swapchain_ = device_.create_swapchain(spec);

            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                command_buffers_[i] = std::make_unique<CommandBuffer>();
            }
        }

        void shutdown() {
            for (auto& cb : command_buffers_) {
                cb.reset();
            }
            swapchain_.reset();
            device_.shutdown();
        }

        void begin_frame() {
            // TODO Später kommt hier der GLFence::wait() hin, um CPU/GPU zu synchronisieren
            command_buffers_[current_frame_index_]->begin();
        }

        void end_frame() {
            command_buffers_[current_frame_index_]->end();
            swapchain_->present();
            current_frame_index_ = (current_frame_index_ + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        void on_window_resize(uint32_t width, uint32_t height) {
            if (swapchain_) {
                swapchain_->resize(width, height);
            }
        }

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
