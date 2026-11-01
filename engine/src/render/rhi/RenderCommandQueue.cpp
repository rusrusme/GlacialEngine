//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/rhi/RenderCommandQueue.hpp"

namespace engine {
    void RenderCommandQueue::submit(RenderCommand command) {
        std::lock_guard lock(queue_mutex_);
        buffer_.emplace_back(std::move(command));
    }

    void RenderCommandQueue::execute() {
        Vector<RenderCommand> local_buffer;
        {
            std::lock_guard lock(queue_mutex_);
            local_buffer.swap(buffer_);
        }
        for (auto& command : local_buffer) {
            if (command) {
                command();
            }
        }
    }

    void RenderCommandQueue::clear() {
        std::lock_guard lock(queue_mutex_);
        buffer_.clear();
    }
}
