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

        virtual void setup(RenderGraphBuilder& builder) = 0;
        virtual void execute(const RenderContext& context, const RenderGraphRegistry& registry) = 0;

        [[nodiscard]] const Vector<std::string>& read_textures() const { return read_textures_; }
        [[nodiscard]] const Vector<std::string>& write_textures() const { return write_textures_; }

        [[nodiscard]] const std::string& name() const { return name_; }
        [[nodiscard]] bool enabled() const { return enabled_; }
        void set_enabled(bool enabled) { enabled_ = enabled; }

    private:
        friend class RenderGraphBuilder;

        std::string name_;
        bool enabled_{true};

        Vector<std::string> read_textures_;
        Vector<std::string> write_textures_;
    };
}
