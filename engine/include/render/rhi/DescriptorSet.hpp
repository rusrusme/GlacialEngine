//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLDescriptorSet.hpp"

namespace engine {
    using DescriptorSet = GLDescriptorSet;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanDescriptorSet.hpp"

namespace engine {
    using DescriptorSet = VulkanDescriptorSet;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12DescriptorSet.hpp"

namespace engine {
    using DescriptorSet = DX12DescriptorSet;
}

#else

#error "No Render API defined"

#endif
