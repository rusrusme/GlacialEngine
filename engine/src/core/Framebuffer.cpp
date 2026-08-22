//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/Framebuffer.hpp"

namespace engine {
    void Framebuffer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);
        glViewport(0, 0, width_, height_);
    }

    void Framebuffer::unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return;
        width_ = width;
        height_ = height;

        invalidate();

        glGenFramebuffers(1, &fbo_id_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);

        // Color Attachment (Textur)
        glGenTextures(1, &color_attachment_id_);
        glBindTexture(GL_TEXTURE_2D, color_attachment_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment_id_, 0);

        // Depth Attachment
        glGenRenderbuffers(1, &depth_attachment_id_);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_attachment_id_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_attachment_id_);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::invalidate() {
        if (fbo_id_) {
            glDeleteFramebuffers(1, &fbo_id_);
            glDeleteTextures(1, &color_attachment_id_);
            glDeleteRenderbuffers(1, &depth_attachment_id_);
        }
    }
}
