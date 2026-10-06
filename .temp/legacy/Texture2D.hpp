//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)
#include "render/legacy/platform/opengl/GLTexture2D.hpp"

namespace engine::legacy {
    using namespace engine;
    using Texture2D = GLTexture2D;
}
#elif defined(RENDER_API_VULKAN)
#include "render/platform/vulkan/VulkanTexture2D.hpp"

namespace engine::legacy {
    using namespace engine;
    using Texture2D = VulkanTexture2D;
}
#else
#error "No Render API defined"
#endif