//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLRendererAPI.hpp"

#include <glad/glad.h>

#include "core/Log.hpp"
#include "core/WindowAPI.hpp"

namespace engine {
    static void GLAPIENTRY gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity,
                                           GLsizei length, const GLchar* message, const void* user_param) {
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        CORE_WARN("[OpenGL Debug] ({0}): {1}", id, message);
    }

    void GLRendererAPI::init() {
        if (!gladLoadGLLoader(window::get_proc_address)) {
            CORE_CRITICAL("[GLRendererAPI] Failed to initialize GLAD");
            return;
        }
        #if defined(DEBUG) || defined(_DEBUG)
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_output, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        #endif

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_MULTISAMPLE);
    }

    void GLRendererAPI::shutdown() {}

    void GLRendererAPI::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void GLRendererAPI::set_clear_color(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void GLRendererAPI::set_clear_color(const float color[4]) {
        glClearColor(color[0], color[1], color[2], color[3]);
    }

    void GLRendererAPI::clear(bool color, bool depth, bool stencil) {
        GLbitfield flags = 0;
        if (color) flags |= GL_COLOR_BUFFER_BIT;
        if (depth) flags |= GL_DEPTH_BUFFER_BIT;
        if (stencil) flags |= GL_STENCIL_BUFFER_BIT;

        glClear(flags);
    }

    void GLRendererAPI::set_depth_test(bool enable) {
        if (enable) {
            glEnable(GL_DEPTH_TEST);
        }
        else {
            glDisable(GL_DEPTH_TEST);
        }
    }

    void GLRendererAPI::set_depth_mask(bool enable) {
        glDepthMask(enable ? GL_TRUE : GL_FALSE);
    }

    void GLRendererAPI::set_cull_face(bool enable, CullMode mode) {
        if (enable) {
            glEnable(GL_CULL_FACE);
            switch (mode) {
                case CullMode::Front : glCullFace(GL_FRONT);
                    break;
                case CullMode::Back : glCullFace(GL_BACK);
                    break;
                case CullMode::FrontAndBack : glCullFace(GL_FRONT_AND_BACK);
                    break;
                default : break;
            }
        }
        else {
            glDisable(GL_CULL_FACE);
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

    void GLRendererAPI::set_polygon_offset(bool enable, float factor, float units) {
        if (enable) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(factor, units);
        }
        else {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    void GLRendererAPI::set_wireframe(bool enable) {
        glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
    }

    void GLRendererAPI::set_seamless_cubemap(bool enable) {
        if (enable) {
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
        else {
            glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
    }
} // engine
