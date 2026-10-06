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