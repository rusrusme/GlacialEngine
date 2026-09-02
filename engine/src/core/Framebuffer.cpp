//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/Framebuffer.hpp"

namespace engine {
    void Framebuffer::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);
        glViewport(0, 0, width_, height_);
    }

    void Framebuffer::unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return;
        width_ = width;
        height_ = height;

        invalidate();

        glGenFramebuffers(1, &fbo_id_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);

        // Color Attachment
        glGenTextures(1, &color_attachment_id_);
        glBindTexture(GL_TEXTURE_2D, color_attachment_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment_id_, 0);

        // Depth Attachment
        glGenTextures(1, &depth_attachment_id_);
        glBindTexture(GL_TEXTURE_2D, depth_attachment_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

        switch (type_) {
            case FramebufferType::ShadowMap: {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

                constexpr float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
            }
            default: {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_attachment_id_, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    uint32_t Framebuffer::get_depth_attachment_id() const {
        return depth_attachment_id_;
    }

    void Framebuffer::invalidate() const {
        if (fbo_id_) {
            glDeleteFramebuffers(1, &fbo_id_);
            glDeleteTextures(1, &color_attachment_id_);
            glDeleteTextures(1, &depth_attachment_id_);
        }
    }
}
