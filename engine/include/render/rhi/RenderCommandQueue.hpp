//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <functional>
#include <mutex>

#include "render/rhi/RHITypes.hpp"

namespace engine {
    using RenderCommand = std::function<void()>;

    class RenderCommandQueue {
    public:
        RenderCommandQueue() = default;
        ~RenderCommandQueue() = default;

        void submit(RenderCommand command);
        void execute();
        void clear();

        [[nodiscard]] std::size_t count() const { return buffer_.size(); }

    private:
        Vector<RenderCommand> buffer_;
        mutable std::mutex queue_mutex_;
    };
}
