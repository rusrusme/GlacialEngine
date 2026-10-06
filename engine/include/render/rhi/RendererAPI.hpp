//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLRendererAPI.hpp"

namespace engine {
    using RendererAPI = GLRendererAPI;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanRendererAPI.hpp"

namespace engine {
    using RendererAPI = VulkanRendererAPI;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12RendererAPI.hpp"

namespace engine {
    using RendererAPI = DX12RendererAPI;
}

#else

#error "No Render API defined"

#endif
