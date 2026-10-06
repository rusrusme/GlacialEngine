//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLShader.hpp"

namespace engine {
    using Shader = GLShader;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanShader.hpp"

namespace engine {
    using Shader = VulkanShader;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12Shader.hpp"

namespace engine {
    using Shader = DX12Shader;
}

#else

#error "No Render API defined"

#endif
