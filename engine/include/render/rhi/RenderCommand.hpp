//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <array>
#include <memory>

#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/RendererAPI.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    class RenderCommand {
    public:
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        static void init() {
            renderer_api_.init();
            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                command_buffers_[i] = std::make_unique<CommandBuffer>();
            }
        }

        static void shutdown() {
            for (auto& cb : command_buffers_) {
                cb.reset();
            }
            renderer_api_.shutdown();
        }

        static void begin_frame() {
            // TODO Später kommt hier der GLFence::wait() hin, um CPU/GPU zu synchronisieren
            command_buffers_[current_frame_index_]->begin();
        }

        static void end_frame() {
            command_buffers_[current_frame_index_]->end();
            current_frame_index_ = (current_frame_index_ + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        [[nodiscard]] static CommandBuffer& active_command_buffer() {
            return *command_buffers_[current_frame_index_];
        }

        [[nodiscard]] static uint32_t current_frame_index() noexcept {
            return current_frame_index_;
        }

    private:
        inline static RendererAPI renderer_api_{};
        inline static std::array<Unique<CommandBuffer>, MAX_FRAMES_IN_FLIGHT> command_buffers_{};
        inline static uint32_t current_frame_index_{0};
    };
}
