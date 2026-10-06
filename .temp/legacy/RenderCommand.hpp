//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

#include "Buffer.hpp"
#include "VertexArray.hpp"

#if defined(RENDER_API_OPENGL)
#include "render/legacy/platform/opengl/GLRendererAPI.hpp"

namespace engine::legacy {
    using namespace engine;
    using RendererAPI = GLRendererAPI;
}
#elif defined(RENDER_API_VULKAN)
#include "render/platform/vulkan/VulkanRendererAPI.hpp"

namespace engine::legacy {
    using namespace engine;
    using RendererAPI = VulkanRendererAPI;
}
#elif defined(RENDER_API_DX12)
#include "render/platform/dx12/DX12RendererAPI.hpp"

namespace engine::legacy {
    using namespace engine;
    using RendererAPI = DX12RendererAPI;
}
#else
#error "No Render API defined"
#endif

namespace engine::legacy {
    using namespace engine;
    class RenderCommand {
    public:
        static void init() {
            RendererAPI::init();
        }

        static void set_viewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
            RendererAPI::set_viewport(x, y, width, height);
        }

        static void set_clear_color(const glm::vec4& color) {
            RendererAPI::set_clear_color(color);
        }

        static void clear(const bool clear_color = true, const bool clear_depth = true) {
            RendererAPI::clear(clear_color, clear_depth);
        }

        // Render States
        static void apply_shader_header(const bshc::BSHHeader& header) {
            RendererAPI::apply_shader_header(header);
        }

        static void set_depth_test(const bool enable) {
            RendererAPI::set_depth_test(enable);
        }

        static void set_depth_mask(const bool enable) {
            RendererAPI::set_depth_mask(enable);
        }

        static void set_cull_face(const bool enable, const bool front_face = false) {
            RendererAPI::set_cull_face(enable, front_face);
        }

        static void set_polygon_offset(const bool enable, const float factor = 0.0f, const float units = 0.0f) {
            RendererAPI::set_polygon_offset(enable, factor, units);
        }

        static void set_blend(const bool enable) {
            RendererAPI::set_blend(enable);
        }

        // Draw Calls
        static void draw_indexed(const VertexArray& vao, const uint32_t index_count = 0) {
            RendererAPI::draw_indexed(vao, index_count);
        }

        static void draw_indexed_instanced(const VertexArray& vao, const uint32_t index_count, const uint32_t instance_count) {
            RendererAPI::draw_indexed_instanced(vao, index_count, instance_count);
        }

        static void draw_indexed_instanced(const VertexArray& vao, const VertexBuffer& instance_vbo, uint32_t index_count, uint32_t instance_count) {
            RendererAPI::draw_indexed_instanced(vao, instance_vbo, index_count, instance_count);
        }

        static void draw_lines(const VertexArray& vao, const uint32_t vertex_count) {
            RendererAPI::draw_lines(vao, vertex_count);
        }

        static void draw_arrays(const VertexArray& vao, const uint32_t vertex_count) {
            RendererAPI::draw_arrays(vao, vertex_count);
        }
    };
}
