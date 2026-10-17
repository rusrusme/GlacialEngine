//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLRendererAPI.hpp"

#include <glad/glad.h>

#include "core/Log.hpp"
#include "core/WindowSystem.hpp"

namespace engine {
    static void GLAPIENTRY gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity,
                                           GLsizei length, const GLchar* message, const void* user_param) {
        // Filter unnecessary OpenGL debug messages (e.g. buffer memory info messages)
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        CORE_WARN("[OpenGL Debug] ({0}): {1}", id, message);
    }

    void GLRendererAPI::init() {
        if (!gladLoadGLLoader(window::proc_address)) {
            CORE_CRITICAL("[GLRendererAPI] Failed to initialize GLAD");
            return;
        }

        #if defined(DEBUG) || defined(_DEBUG)
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_output, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        #endif

        // Global driver features, which hold for the whole application runtime
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_MULTISAMPLE);
    }

    void GLRendererAPI::shutdown() { /* Placeholder for future */
    }
}
