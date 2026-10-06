//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLSwapchain.hpp"

namespace engine {
    using Swapchain = GLSwapchain;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanSwapchain.hpp"

namespace engine {
    using Swapchain = VulkanSwapchain;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12Swapchain.hpp"

namespace engine {
    using Swapchain = DX12Swapchain;
}

#else

#error "No Render API defined"

#endif
