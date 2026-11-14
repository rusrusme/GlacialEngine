//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/rhi/RenderContext.hpp"

#include "core/Window.hpp"

namespace engine {
    void RenderContext::init(const Window& window) {
        device_.init();

        SwapchainSpec spec;
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

    void RenderContext::shutdown() {
        for (auto& cb : command_buffers_) {
            cb.reset();
        }
        swapchain_.reset();
        device_.shutdown();
    }

    void RenderContext::begin_frame() {
        // TODO Später kommt hier der GLFence::wait() hin, um CPU/GPU zu synchronisieren
        command_buffers_[current_frame_index_]->begin();
    }

    void RenderContext::end_frame() {
        command_buffers_[current_frame_index_]->end();
        swapchain_->present();
        current_frame_index_ = (current_frame_index_ + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void RenderContext::on_window_resize(uint32_t width, uint32_t height) {
        if (swapchain_) {
            swapchain_->resize(width, height);
        }
    }
}
