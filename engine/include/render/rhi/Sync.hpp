//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLSync.hpp"

namespace engine {
    using Fence = GLFence;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanSync.hpp"

namespace engine {
    using Fence = VulkanFence;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12Sync.hpp"

namespace engine {
    using Fence = DX12Fence;
}

#else

#error "No Render API defined"

#endif
