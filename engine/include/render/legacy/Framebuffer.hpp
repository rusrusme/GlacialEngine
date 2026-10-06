//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)
#include "render/legacy/platform/opengl/GLFramebuffer.hpp"

namespace engine {
    using Framebuffer = GLFramebuffer;
    using FramebufferType = GLFramebufferType;
}
#elif defined(RENDER_API_VULKAN)
#include "render/platform/vulkan/VulkanFramebuffer.hpp"

namespace engine {
    using Framebuffer = VulkanFramebuffer;
    using FramebufferType = VulkanFramebufferType;
}
#else
#error "No Render API defined"
#endif
