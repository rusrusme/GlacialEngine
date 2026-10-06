//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/BufferLayout.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Shader.hpp"

namespace engine {
    struct PipelineStateDesc {
        Shader shader;
        BufferLayout vertex_layout;

        CullMode cull_mode = CullMode::Back;
        PolygonMode polygon_mode = PolygonMode::Fill;

        bool depth_test = true;
        bool depth_write = true;
        CompareOp depth_compare = CompareOp::Less;

        bool blend_enable = false;
    };
}

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLPipelineState.hpp"

namespace engine {
    using PipelineState = GLPipelineState;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanPipelineState.hpp"

namespace engine {
    using PipelineState = VulkanPipelineState;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12PipelineState.hpp"

namespace engine {
    using PipelineState = DX12PipelineState;
}

#else

#error "No Render API defined"

#endif
