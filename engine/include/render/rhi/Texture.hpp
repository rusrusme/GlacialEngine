//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLTexture.hpp"

namespace engine {
    using Texture2D = GLTexture2D;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanTexture.hpp"

namespace engine {
    using Texture2D = VulkanTexture2D;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12Texture.hpp"

namespace engine {
    using Texture2D = DX12Texture2D;
}

#else

#error "No Render API defined"

#endif
