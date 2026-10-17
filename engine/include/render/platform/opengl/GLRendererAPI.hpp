//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

namespace engine {
    class GLRendererAPI {
    public:
        GLRendererAPI() = default;
        ~GLRendererAPI() = default;

        void init();
        void shutdown();

        [[nodiscard]] static constexpr RendererBackend backend() noexcept { return RendererBackend::OpenGL; }
    };
}
