//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/BufferLayout.hpp"
#include "render/rhi/DescriptorSetLayout.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Shader.hpp"

namespace engine {
    // The render targets a graphics pipeline will be used with. Vulkan bakes these
    // into the VkGraphicsPipeline (renderPass/compatible render pass) and DX12 into
    // the PSO (RTVFormats/DSVFormat/SampleDesc) — both are required at creation time.
    // OpenGL derives its targets from the bound framebuffer and only validates.
    struct RenderTargetState {
        Vector<TextureFormat> color_formats{};
        TextureFormat depth_format{TextureFormat::None};
        uint32_t sample_count{1};
        bool has_depth{false};

        [[nodiscard]] bool empty() const { return color_formats.empty() && !has_depth; }
    };

    struct ComputePipelineDesc {
        Shared<Shader> shader{nullptr};
        PipelineLayoutDesc pipeline_layout{};
    };

    struct PipelineStateDesc {
        Shared<Shader> shader{nullptr};
        BufferLayout vertex_layout;

        PipelineLayoutDesc pipeline_layout{};
        RenderTargetState render_target{};

        CullMode cull_mode = CullMode::Back;
        PolygonMode polygon_mode = PolygonMode::Fill;

        bool depth_test = true;
        bool depth_write = true;
        CompareOp depth_compare = CompareOp::Less;

        bool blend_enable = false;
    };
}
