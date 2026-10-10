//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/ComputePipelineDesc.hpp"

#include <cstdint>

namespace engine {
    class GLIndirectBuffer;

    class GLComputePipeline {
    public:
        GLComputePipeline() = default;
        explicit GLComputePipeline(ComputePipelineDesc desc);
        ~GLComputePipeline() = default;

        GLComputePipeline(const GLComputePipeline&) = delete;
        GLComputePipeline& operator=(const GLComputePipeline&) = delete;
        GLComputePipeline(GLComputePipeline&&) noexcept = default;
        GLComputePipeline& operator=(GLComputePipeline&&) noexcept = default;

        void bind() const;
        void dispatch(uint32_t group_x, uint32_t group_y = 1, uint32_t group_z = 1) const;
        void dispatch_indirect(const GLIndirectBuffer& indirect_buffer, size_t offset = 0) const;

        static void set_memory_barrier(MemoryBarrierFlags flags = MemoryBarrierFlags::All);

        [[nodiscard]] const ComputePipelineDesc& description() const noexcept { return desc_; }

    private:
        ComputePipelineDesc desc_{};
    };
}
