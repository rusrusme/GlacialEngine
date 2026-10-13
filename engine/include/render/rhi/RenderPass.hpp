//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#include <cstdint>

namespace engine {
    // What to do with an attachment's previous contents when a render pass starts.
    // OpenGL maps these onto glClear / attachment clears; Vulkan and DX12 map them
    // onto loadOp / D3D12_LOAD_ACTION respectively.
    enum class AttachmentLoadOp : uint32_t {
        Load     = 0,
        Clear    = 1,
        DontCare = 2
    };

    // What to do with an attachment's contents when a render pass ends. DX12
    // needs this to decide whether to keep the resource in its compressed state.
    enum class AttachmentStoreOp : uint32_t {
        Store    = 0,
        DontCare = 1
    };

    enum class AttachmentType : uint32_t {
        Color        = 0,
        Depth        = 1,
        DepthStencil = 2
    };

    struct RenderPassAttachmentDesc {
        TextureFormat format{TextureFormat::None};
        AttachmentType type{AttachmentType::Color};
        AttachmentLoadOp load_op{AttachmentLoadOp::Clear};
        AttachmentStoreOp store_op{AttachmentStoreOp::Store};
        bool is_swapchain_image{false};
    };

    struct ClearValue {
        float color[4]{0.0f, 0.0f, 0.0f, 1.0f};
        float depth{1.0f};
        uint32_t stencil{0};
    };

    // Platform-neutral render pass description: per-attachment format, type and
    // load/store behaviour plus the sample count. OpenGL resolves this against the
    // framebuffer it is bound to; Vulkan/DX12 build a VkRenderPass or a render
    // target state object from it.
    struct RenderPassDesc {
        Vector<RenderPassAttachmentDesc> attachments{};
        uint32_t samples{1};

        [[nodiscard]] bool clears_any_attachment() const {
            for (const RenderPassAttachmentDesc& attachment : attachments) {
                if (attachment.load_op == AttachmentLoadOp::Clear) {
                    return true;
                }
            }
            return false;
        }
    };
}
