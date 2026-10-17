//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLDevice.hpp"

namespace engine {
    using RHIDevice = GLDevice;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanDevice.hpp"

namespace engine {
    using RHIDevice = VulkanDevice;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12Device.hpp"

namespace engine {
    using RHIDevice = DX12Device;
}

#else

#error "No Render API defined"

#endif
