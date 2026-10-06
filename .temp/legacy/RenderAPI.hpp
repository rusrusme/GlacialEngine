//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine::legacy {
    using namespace engine;
    enum class RenderAPI {
        None = 0,
        OpenGL,
        Vulkan,
        DX12
    };

    constexpr RenderAPI api() {
        #if defined(RENDER_API_OPENGL)
        return RenderAPI::OpenGL;
        #elif defined(RENDER_API_VULKAN)
        return RenderAPI::Vulkan;
        #elif defined(RENDER_API_DX12)
        return RenderAPI::DX12;
        #else
        return RenderAPI::None;
        #endif
    }
}
