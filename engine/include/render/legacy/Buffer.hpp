//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)
#include "render/legacy/platform/opengl/GLBuffer.hpp"

namespace engine {
    using VertexBuffer = GLVertexBuffer;
    using IndexBuffer = GLIndexBuffer;
}
#elif defined(RENDER_API_VULKAN)
#include "render/platform/vulkan/VulkanBuffer.hpp"

namespace engine {
    using VertexBuffer = VulkanVertexBuffer;
    using IndexBuffer = VulkanIndexBuffer;
}
#else
#error "No Render API defined"
#endif
