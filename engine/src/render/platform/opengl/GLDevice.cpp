//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

// ReSharper disable CppMemberFunctionMayBeStatic
#include "render/platform/opengl/GLDevice.hpp"

#include <glad/glad.h>

#include "core/Log.hpp"
#include "core/WindowSystem.hpp"
#include "render/platform/opengl/GLBuffer.hpp"
#include "render/platform/opengl/GLFramebuffer.hpp"
#include "render/platform/opengl/GLPipelineState.hpp"
#include "render/platform/opengl/GLSwapchain.hpp"

namespace engine {
    static void GLAPIENTRY gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity,
                                           GLsizei length, const GLchar* message, const void* user_param) {
        // Filter unnecessary OpenGL debug messages (e.g. buffer memory info messages)
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        CORE_WARN("[OpenGL Debug] ({0}): {1}", id, message);
    }

    void GLDevice::init() {
        if (!gladLoadGLLoader(window::proc_address)) {
            CORE_CRITICAL("[GLDevice] Failed to initialize GLAD");
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

    void GLDevice::shutdown() { /* Placeholder for future */ }

    // Factory Implementationen
    Unique<GLVertexBuffer> GLDevice::create_vertex_buffer(size_t size, const void* data) const {
        return std::make_unique<GLVertexBuffer>(size, data);
    }

    Unique<GLIndexBuffer> GLDevice::create_index_buffer(size_t size, const uint32_t* indices) const {
        return std::make_unique<GLIndexBuffer>(size, indices);
    }

    Unique<GLUniformBuffer> GLDevice::create_uniform_buffer(size_t size, uint32_t binding) const {
        return std::make_unique<GLUniformBuffer>(size, binding);
    }

    Unique<GLShaderStorageBuffer> GLDevice::create_shader_storage_buffer(size_t size, uint32_t binding) const {
        return std::make_unique<GLShaderStorageBuffer>(size, binding);
    }

    Unique<GLIndirectBuffer> GLDevice::create_indirect_buffer(size_t size) const {
        return std::make_unique<GLIndirectBuffer>(size);
    }

    Unique<GLPipelineState> GLDevice::create_pipeline_state(const PipelineStateDesc& desc) const {
        return std::make_unique<GLPipelineState>(desc);
    }

    Unique<GLFramebuffer> GLDevice::create_framebuffer(const FramebufferSpecification& spec) const {
        return std::make_unique<GLFramebuffer>(spec);
    }

    Shared<GLSwapchain> GLDevice::create_swapchain(const SwapchainSpec& spec) const {
        return std::make_shared<GLSwapchain>(spec);
    }

    Unique<GLDescriptorSet> GLDevice::create_descriptor_set() const {
        return std::make_unique<GLDescriptorSet>();
    }

    Unique<GLTexture2D> GLDevice::create_texture_2d(const TextureSpecification& spec, const void* data = nullptr) const {
        if (data) {
            const size_t size = spec.width * spec.height * 4;
            return std::make_unique<GLTexture2D>(spec, data, size);
        }
        return std::make_unique<GLTexture2D>(spec);
    }
}
