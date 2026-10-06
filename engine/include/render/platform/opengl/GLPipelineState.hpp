//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/PipelineStateSpec.hpp"

#include <cstdint>

namespace engine {
    class GLPipelineState {
    public:
        GLPipelineState() = default;
        explicit GLPipelineState(PipelineStateDesc  desc);
        ~GLPipelineState();

        GLPipelineState(const GLPipelineState&) = delete;
        GLPipelineState& operator=(const GLPipelineState&) = delete;

        GLPipelineState(GLPipelineState&& other) noexcept;
        GLPipelineState& operator=(GLPipelineState&& other) noexcept;

        void bind() const;

        [[nodiscard]] uint32_t vao_id() const noexcept { return vao_id_; }
        [[nodiscard]] const PipelineStateDesc& description() const noexcept { return desc_; }

    private:
        void setup_vertex_array();
        void release();

        PipelineStateDesc desc_{};
        uint32_t vao_id_{0};
    };
} // engine
