//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

#include "render/rhi/CommandBuffer.hpp"

namespace engine {
    class RenderGraphBuilder;
    class RenderGraphRegistry;

    /**
     * @brief Context passed to render passes during frame execution.
     */
    struct RenderContext {
        CommandBuffer& command_buffer;
        uint32_t current_frame_index{0};
    };

    /**
     * @brief Abstract base class for all CPU/GPU render passes.
     */
    class RenderPass {
    public:
        explicit RenderPass(std::string pass_name) : name_(std::move(pass_name)) {}
        virtual ~RenderPass() = default;

        /**
         * @brief Declares resource reads, writes, and transient attachment creation.
         */
        virtual void setup(RenderGraphBuilder& builder) = 0;

        /**
         * @brief Records backend command execution into the provided CommandBuffer.
         */
        virtual void execute(const RenderContext& context, const RenderGraphRegistry& registry) = 0;

        [[nodiscard]] const std::string& name() const { return name_; }
        [[nodiscard]] bool is_enabled() const { return enabled_; }
        void set_enabled(bool enabled) { enabled_ = enabled; }

    private:
        std::string name_;
        bool enabled_{true};
    };
}
