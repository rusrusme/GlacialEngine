//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)
#include "render/platform/opengl/GLVertexArray.hpp"

namespace engine {
    using VertexArray = GLVertexArray;
}
#elif defined(RENDER_API_VULKAN)
#include "render/platform/vulkan/VulkanVertexArray.hpp"

namespace engine {
    using VertexArray = VulkanVertexArray;
}
#else
#error "No Render API defined"
#endif
