//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#include <vector>

namespace engine {
    struct FramebufferAttachmentSpec {
        TextureFormat format = TextureFormat::None;

        FramebufferAttachmentSpec() = default;
        FramebufferAttachmentSpec(TextureFormat format) : format(format) {}
    };

    struct FramebufferSpecification {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t samples = 1;
        std::vector<FramebufferAttachmentSpec> attachments;
        bool swapchain_target = false;
    };
}