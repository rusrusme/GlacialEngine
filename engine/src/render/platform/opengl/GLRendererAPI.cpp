//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLRendererAPI.hpp"

#include "core/Log.hpp"
#include "core/WindowAPI.hpp"
#include "render/mesh/InstanceData.hpp"
#include "render/platform/opengl/GLBuffer.hpp"
#include "render/platform/opengl/GLVertexArray.hpp"

namespace engine {
    void GLRendererAPI::init() {
        if (!gladLoadGLLoader(window::get_proc_address)) {
            CORE_CRITICAL("GLRendererAPI: Failed to initialize GLAD");
            return;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    void GLRendererAPI::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void GLRendererAPI::set_clear_color(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void GLRendererAPI::clear(bool clear_color, bool clear_depth) {
        GLbitfield mask = 0;
        if (clear_color) mask |= GL_COLOR_BUFFER_BIT;
        if (clear_depth) mask |= GL_DEPTH_BUFFER_BIT;
        glClear(mask);
    }

    void GLRendererAPI::dispatch_compute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) {
        glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
            GL_TEXTURE_FETCH_BARRIER_BIT |
            GL_SHADER_STORAGE_BARRIER_BIT |
            GL_BUFFER_UPDATE_BARRIER_BIT);
    }

    void GLRendererAPI::apply_shader_header(const bshc::BSHHeader& header) {
        // Depth Test
        set_depth_test(header.depth_test != 0);

        // Depth Mask
        set_depth_mask(header.depth_write != 0);

        // Cull Mode (0 = None, 1 = Front, 2 = Back)
        if (header.cull_mode == 0) {
            set_cull_face(false);
        }
        else {
            set_cull_face(true, header.cull_mode == 1);
        }

        // Blend Mode (0 = Opaque, 1 = Alpha, 2 = Additive)
        if (header.blend_mode == 0) {
            set_blend(false);
        }
        else {
            glEnable(GL_BLEND);
            if (header.blend_mode == 1) { // Alpha
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else if (header.blend_mode == 2) { // Additive
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
        }
    }

    void GLRendererAPI::set_depth_test(bool enable) {
        if (enable)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void GLRendererAPI::set_depth_mask(bool enable) {
        glDepthMask(enable ? GL_TRUE : GL_FALSE);
    }

    void GLRendererAPI::set_cull_face(bool enable, bool front_face) {
        if (enable) {
            glEnable(GL_CULL_FACE);
            glCullFace(front_face ? GL_FRONT : GL_BACK);
        }
        else {
            glDisable(GL_CULL_FACE);
        }
    }

    void GLRendererAPI::set_polygon_offset(bool enable, float factor, float units) {
        if (enable) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(factor, units);
        }
        else {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    void GLRendererAPI::set_blend(bool enable) {
        if (enable) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else {
            glDisable(GL_BLEND);
        }
    }

    void GLRendererAPI::set_seamless_cubemap(bool enable) {
        if (enable)
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        else
            glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    void GLRendererAPI::draw_indexed(const GLVertexArray& vao, uint32_t index_count) {
        vao.bind();
        uint32_t count = index_count ? index_count : vao.get_index_buffer()->get_count();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, nullptr);
        vao.unbind();
    }

    void GLRendererAPI::draw_indexed_instanced(const GLVertexArray& vao, uint32_t index_count, uint32_t instance_count) {
        vao.bind();
        uint32_t count = index_count ? index_count : vao.get_index_buffer()->get_count();
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instance_count));
        vao.unbind();
    }

    void GLRendererAPI::draw_indexed_instanced(const GLVertexArray& vao, const GLVertexBuffer& instance_vbo, uint32_t index_count, uint32_t instance_count) {
        vao.bind();
        instance_vbo.bind();

        constexpr auto stride = static_cast<GLsizei>(sizeof(InstanceData));

        // Instance Transform Matrix (Attributes 4, 5, 6, 7)
        for (uint32_t i = 0; i < 4; ++i) {
            glEnableVertexAttribArray(4 + i);
            glVertexAttribPointer(
                4 + i,
                4,
                GL_FLOAT,
                GL_FALSE,
                stride,
                reinterpret_cast<const void*>(sizeof(glm::vec4) * i)
            );
            glVertexAttribDivisor(4 + i, 1);
        }

        // Instance Color (Attribute 8)
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(
            8,
            4,
            GL_FLOAT,
            GL_FALSE,
            stride,
            reinterpret_cast<const void*>(offsetof(InstanceData, color))
        );
        glVertexAttribDivisor(8, 1);

        // Instance PBR Properties (Attribute 9)
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(
            9,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            reinterpret_cast<const void*>(offsetof(InstanceData, pbr))
        );
        glVertexAttribDivisor(9, 1);

        const uint32_t count = index_count ? index_count : vao.get_index_buffer()->get_count();
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instance_count));

        vao.unbind();
    }

    void GLRendererAPI::draw_lines(const GLVertexArray& vao, uint32_t vertex_count) {
        vao.bind();
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertex_count));
        vao.unbind();
    }

    void GLRendererAPI::draw_arrays(const GLVertexArray& vao, uint32_t vertex_count) {
        vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertex_count));
        vao.unbind();
    }

    void GLRendererAPI::unbind_texture(uint32_t slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void GLRendererAPI::unbind_image(uint32_t unit) {
        glBindImageTexture(unit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    }
}
