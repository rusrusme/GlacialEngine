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

        void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void set_clear_color(const glm::vec4& color);
        void set_clear_color(const float color[4]);
        void clear(bool color = true, bool depth = true, bool stencil = false);

        void set_depth_test(bool enable);
        void set_depth_mask(bool enable);
        void set_cull_face(bool enable, CullMode mode = CullMode::Back);
        void set_blend(bool enable);
        void set_polygon_offset(bool enable, float factor = 0.0f, float units = 0.0f);
        void set_wireframe(bool enable);
        void set_seamless_cubemap(bool enable);

    private:
        float clear_color_[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    };
} // engine
