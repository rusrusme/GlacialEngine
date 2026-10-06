//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#include <cstdint>

namespace engine {
    struct TextureSpecification {
        uint32_t width = 1;
        uint32_t height = 1;
        TextureFormat format = TextureFormat::RGBA8;
        TextureFilter min_filter = TextureFilter::Linear;
        TextureFilter mag_filter = TextureFilter::Linear;
        TextureWrap wrap_s = TextureWrap::Repeat;
        TextureWrap wrap_t = TextureWrap::Repeat;
        bool generate_mipmaps = true;
    };
}

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
