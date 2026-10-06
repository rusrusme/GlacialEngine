//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLCommandBuffer.hpp"

namespace engine {
    using CommandBuffer = GLCommandBuffer;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanCommandBuffer.hpp"

namespace engine {
    using CommandBuffer = VulkanCommandBuffer;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12CommandBuffer.hpp"

namespace engine {
    using CommandBuffer = DX12CommandBuffer;
}

#else

#error "No Render API defined"

#endif
