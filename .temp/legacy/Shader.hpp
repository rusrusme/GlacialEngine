//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)
#include "render/legacy/platform/opengl/GLShader.hpp"

namespace engine::legacy {
    using namespace engine;
    using Shader = GLShader;
}
#elif defined(RENDER_API_VULKAN)
#include "render/platform/vulkan/VulkanShader.hpp"

namespace engine::legacy {
    using namespace engine;
    using Shader = VulkanShader;
}
#else
#error "No Render API defined"
#endif
