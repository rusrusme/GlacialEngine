//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLSync.hpp"

#include <utility>

namespace engine {
    GLFence::~GLFence() {
        release();
    }

    GLFence::GLFence(GLFence&& other) noexcept
        : sync_obj_(other.sync_obj_) {
        other.sync_obj_ = nullptr;
    }

    GLFence& GLFence::operator=(GLFence&& other) noexcept {
        if (this != &other) {
            release();
            sync_obj_ = other.sync_obj_;
            other.sync_obj_ = nullptr;
        }
        return *this;
    }

    void GLFence::signal() {
        reset();
        sync_obj_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    bool GLFence::wait(uint64_t timeout_ns) {
        if (!sync_obj_) {
            return true;
        }
        const GLenum result = glClientWaitSync(
            sync_obj_,
            GL_SYNC_FLUSH_COMMANDS_BIT,
            static_cast<GLuint64>(timeout_ns)
        );
        return result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED;
    }

    void GLFence::gpu_wait() {
        if (!sync_obj_) {
            return;
        }
        glWaitSync(sync_obj_, 0, GL_TIMEOUT_IGNORED);
    }

    void GLFence::reset() {
        release();
    }

    bool GLFence::is_signaled() const {
        if (!sync_obj_) {
            return true;
        }
        GLint value = GL_UNSIGNALED;
        GLsizei length = 0;
        glGetSynciv(sync_obj_, GL_SYNC_STATUS, sizeof(GLint), &length, &value);
        return value == GL_SIGNALED;
    }

    void GLFence::release() {
        if (sync_obj_) {
            glDeleteSync(sync_obj_);
            sync_obj_ = nullptr;
        }
    }
}
