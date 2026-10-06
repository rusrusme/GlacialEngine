//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include "render/rhi/Shader.hpp"

namespace engine {
    struct ComputePipelineDesc {
        Shader shader;
    };
}

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLComputePipeline.hpp"

namespace engine {
    using ComputePipeline = GLComputePipeline;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanComputePipeline.hpp"

namespace engine {
    using ComputePipeline = VulkanComputePipeline;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12ComputePipeline.hpp"

namespace engine {
    using ComputePipeline = DX12ComputePipeline;
}

#else

#error "No Render API defined"

#endif
