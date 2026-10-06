//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

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
