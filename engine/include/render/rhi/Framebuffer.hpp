//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLFramebuffer.hpp"

namespace engine {
    using Framebuffer = GLFramebuffer;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanFramebuffer.hpp"

namespace engine {
    using Framebuffer = VulkanFramebuffer;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12Framebuffer.hpp"

namespace engine {
    using Framebuffer = DX12Framebuffer;
}

#else

#error "No Render API defined"

#endif
