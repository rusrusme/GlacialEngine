//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/BufferLayout.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Shader.hpp"

namespace engine {
    struct ComputePipelineDesc {
        Shared<Shader> shader{nullptr};
    };

    struct PipelineStateDesc {
        Shared<Shader> shader{nullptr};
        BufferLayout vertex_layout;

        CullMode cull_mode = CullMode::Back;
        PolygonMode polygon_mode = PolygonMode::Fill;

        bool depth_test = true;
        bool depth_write = true;
        CompareOp depth_compare = CompareOp::Less;

        bool blend_enable = false;
    };
}
