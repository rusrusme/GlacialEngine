//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    enum class RenderAPI {
        None = 0,
        OpenGL,
        Vulkan
    };

    class RendererAPI {
    public:
        static constexpr RenderAPI api() {
            #if defined(RENDER_API_OPENGL)
            return RenderAPI::OpenGL;
            #elif defined(RENDER_API_VULKAN)
            return RenderAPI::Vulkan;
            #else
            return RenderAPI::None;
            #endif
        }
    };
}