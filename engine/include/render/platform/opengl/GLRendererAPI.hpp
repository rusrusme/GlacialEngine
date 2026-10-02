//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <BSH.hpp>
#include <glm/glm.hpp>

namespace engine {
    class GLVertexBuffer;
    class GLVertexArray;

    class GLRendererAPI {
    public:
        static void init();
        static void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        static void set_clear_color(const glm::vec4& color);
        static void clear(bool clear_color = true, bool clear_depth = true);

        // Render States
        static void apply_shader_header(const bshc::BSHHeader& header);

        static void set_depth_test(bool enable);
        static void set_depth_mask(bool enable);
        static void set_cull_face(bool enable, bool front_face = false);
        static void set_polygon_offset(bool enable, float factor = 0.0f, float units = 0.0f);
        static void set_blend(bool enable);
        static void set_seamless_cubemap(bool enable);

        // Draw Calls
        static void draw_indexed(const GLVertexArray& vao, uint32_t index_count = 0);
        static void draw_indexed_instanced(const GLVertexArray& vao, uint32_t index_count, uint32_t instance_count);
        static void draw_indexed_instanced(const GLVertexArray& vao, const GLVertexBuffer& instance_vbo, uint32_t index_count, uint32_t instance_count);
        static void draw_lines(const GLVertexArray& vao, uint32_t vertex_count);
        static void draw_arrays(const GLVertexArray& vao, uint32_t vertex_count);
    };
}