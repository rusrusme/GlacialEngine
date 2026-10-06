//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLBuffer.hpp"

namespace engine {
    using VertexBuffer = GLVertexBuffer;
    using IndexBuffer = GLIndexBuffer;
    using UniformBuffer = GLUniformBuffer;
    using ShaderStorageBuffer = GLShaderStorageBuffer;
    using IndirectBuffer = GLIndirectBuffer;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanBuffer.hpp"

namespace engine {
    // Todo: Implement Vulkan
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12Buffer.hpp"

namespace engine {
    // Todo: Implement DX12
}

#else

#error "No Render API defined"

#endif
