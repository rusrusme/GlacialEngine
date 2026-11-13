//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "glacialc/Compiler.hpp"

#include <memory>

#include "glacialc/Log.hpp"
#include "glacialc/platform/dx12/DX12Compiler.hpp"
#include "glacialc/platform/opengl/GLCompiler.hpp"
#include "glacialc/platform/vulkan/VulkanCompiler.hpp"

namespace glacialc {
    Unique<Compiler> Compiler::create(const RenderBackend& render_backend) {
        switch (render_backend) {
                #if defined(GLACIALC_BACKEND_OPENGL)
            case RenderBackend::OpenGL :
                return std::make_unique<GLCompiler>();
                #endif

                #if defined(GLACIALC_BACKEND_VULKAN)
            case RenderBackend::Vulkan :
                return std::make_unique<VulkanCompiler>();
                #endif

                #if defined(GLACIALC_BACKEND_DX12)
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
