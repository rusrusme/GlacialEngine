//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Compiler.hpp"

#include <memory>

#include "Internal.hpp"
#include "gshader/Log.hpp"
#include "gshader/platform/dx12/DX12Compiler.hpp"
#include "gshader/platform/opengl/GLCompiler.hpp"
#include "gshader/platform/vulkan/VulkanCompiler.hpp"

namespace gshader {
    Unique<Compiler> Compiler::create(const RenderBackend& render_backend) {
        switch (render_backend) {
                #if defined(GSHADER_ENABLE_OPENGL)
            case RenderBackend::OpenGL :
                return std::make_unique<GLCompiler>();
                #endif

                #if defined(GSHADER_ENABLE_VULKAN)
            case RenderBackend::Vulkan :
                return std::make_unique<VulkanCompiler>();
                #endif

                #if defined(GSHADER_ENABLE_DX12)
            case RenderBackend::DX12 :
                return std::make_unique<DX12Compiler>();
                #endif

            default :
                break;
        }
        GSH_ERROR("Render backend '{}' is disabled or unsupported in this build.", to_string(render_backend));
        return nullptr;
    }
}
