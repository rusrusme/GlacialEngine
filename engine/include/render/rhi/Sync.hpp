//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine {
    enum class BarrierFlags : uint32_t {
        None               = 0,
        IndirectCommandBit = 1 << 0, // Wait for compute to write ICB
        ShaderStorageBit   = 1 << 1, // Wait for SSBO writes (Light List)
        TextureFetchBit    = 1 << 2, // Wait for depth map / HZB writes
        All                = 0xFFFFFFFF
    };
}

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
