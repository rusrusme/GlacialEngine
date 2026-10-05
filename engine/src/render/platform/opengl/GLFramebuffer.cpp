//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLFramebuffer.hpp"

namespace engine {
    void GLFramebuffer::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);
        glViewport(0, 0, width_, height_);
    }

    void GLFramebuffer::unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GLFramebuffer::bind_color_texture(const uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, color_attachment_id_);
    }

    void GLFramebuffer::bind_color_image(const uint32_t unit, const uint32_t level, const bool is_write_only) const {
        glBindImageTexture(
            unit,
            color_attachment_id_,
            level,
            GL_FALSE,
            0,
            is_write_only ? GL_WRITE_ONLY : GL_READ_ONLY,
            GL_R32F
        );
    }

    void GLFramebuffer::bind_depth_texture(const uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(type_ == GLFramebufferType::PointShadowMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, depth_attachment_id_);
    }

    void GLFramebuffer::bind_depth_image(uint32_t unit, uint32_t level, bool is_write_only) const {
        glBindImageTexture(
            unit,
            depth_attachment_id_,
            level,
            GL_FALSE,
            0,
            is_write_only ? GL_WRITE_ONLY : GL_READ_ONLY,
            GL_R32F // Using 32-bit float layout for compute image access
        );
    }

    void GLFramebuffer::resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return;
        width_ = width;
        height_ = height;

        invalidate();

        glGenFramebuffers(1, &fbo_id_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);

        // Color Attachment. Skipped for the point-light shadow map (a depth-only,
        // layered cube map): attaching a 2D color texture alongside a layered
        // cube-map depth attachment makes the FBO GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,
        // so nothing renders.
        if (type_ != GLFramebufferType::PointShadowMap && type_ != GLFramebufferType::DepthOnly) {
            glGenTextures(1, &color_attachment_id_);
            glBindTexture(GL_TEXTURE_2D, color_attachment_id_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment_id_, 0);
        }

        // Depth Attachment
        switch (type_) {
            case GLFramebufferType::ShadowMap : {
                glGenTextures(1, &depth_attachment_id_);
                glBindTexture(GL_TEXTURE_2D, depth_attachment_id_);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

                constexpr float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_attachment_id_, 0);
                break;
            }
            // Point-light cubemap depth map. The whole cubemap is attached as a
            // single layered texture so a geometry shader can route writes to any
            // face via gl_Layer.
            case GLFramebufferType::PointShadowMap : {
                glGenTextures(1, &depth_attachment_id_);
                glBindTexture(GL_TEXTURE_CUBE_MAP, depth_attachment_id_);

                for (uint32_t i = 0; i < 6; ++i) {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
                }

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_attachment_id_, 0);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);
                break;
            }
            case GLFramebufferType::DepthOnly : {
                mip_levels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

                glGenTextures(1, &color_attachment_id_);
                glBindTexture(GL_TEXTURE_2D, color_attachment_id_);
                glTexStorage2D(GL_TEXTURE_2D, mip_levels_, GL_R32F, width_, height_);

                // WICHTIG: Explicit Base/Max Level & NEAREST setzen, damit OpenGL die Textur sofort als "complete" akzeptiert!
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(mip_levels_ - 1));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment_id_, 0);

                // 2. Depth Attachment für Depth-Testing
                glGenTextures(1, &depth_attachment_id_);
                glBindTexture(GL_TEXTURE_2D, depth_attachment_id_);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment_id_, 0);

                const GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
                glDrawBuffers(1, draw_buffers);
                glReadBuffer(GL_COLOR_ATTACHMENT0);
                break;
            }
            default : {
                glGenTextures(1, &depth_attachment_id_);
                glBindTexture(GL_TEXTURE_2D, depth_attachment_id_);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_attachment_id_, 0);
                break;
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    uint32_t GLFramebuffer::get_depth_attachment_id() const {
        return depth_attachment_id_;
    }

    void GLFramebuffer::invalidate() const {
        if (fbo_id_) {
            glDeleteFramebuffers(1, &fbo_id_);
            glDeleteTextures(1, &color_attachment_id_);
            glDeleteTextures(1, &depth_attachment_id_);
        }
    }
}
