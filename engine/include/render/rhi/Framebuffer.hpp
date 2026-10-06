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
        explicit FramebufferAttachmentSpec(TextureFormat format) : format(format) {}
    };

    struct FramebufferSpecification {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t samples = 1;
        std::vector<FramebufferAttachmentSpec> attachments;
        bool swapchain_target = false;
    };
}

#if defined(RENDER_API_OPENGL)

#include "render/platform/opengl/GLFramebuffer.hpp"

namespace engine {
    using Framebuffer = GLFramebuffer;
}

#elif defined(RENDER_API_VULKAN)

#include "render/platform/vulkan/VulkanFramebuffer.hpp"

namespace engine {
    using Framebuffer = VulkanFramebuffer;
}

#elif defined(RENDER_API_DX12)

#include "render/platform/dx12/DX12Framebuffer.hpp"

namespace engine {
    using Framebuffer = DX12Framebuffer;
}

#else

#error "No Render API defined"

#endif
