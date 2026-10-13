//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <functional>
#include <mutex>

#include "render/rhi/RHITypes.hpp"

namespace engine {
    using RenderCommandFun = std::function<void()>;

    class RenderCommandQueue {
    public:
        RenderCommandQueue() = default;
        ~RenderCommandQueue() = default;

        void submit(RenderCommandFun command);
        void execute();
        void clear();

        [[nodiscard]] std::size_t command_count() const { return command_buffer_.size(); }

    private:
        Vector<RenderCommandFun> command_buffer_;
        mutable std::mutex queue_mutex_;
    };
}
