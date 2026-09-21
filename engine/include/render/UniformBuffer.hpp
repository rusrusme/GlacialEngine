//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)
#include "render/platform/opengl/GLUniformBuffer.hpp"

namespace engine {
    using UniformBuffer = GLUniformBuffer;
}
#elif defined(RENDER_API_VULKAN)
#include "render/platform/vulkan/VulkanUniformBuffer.hpp"

namespace engine {
    using UniformBuffer = VulkanUniformBuffer;
}
#else
#error "No Render API defined"
#endif
