//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLFramebuffer.hpp"

#include "core/Log.hpp"

#include <utility>
#include <glad/glad.h>

namespace engine {
    static bool is_depth_format(TextureFormat format) {
        switch (format) {
            case TextureFormat::Depth24Stencil8 :
            case TextureFormat::Depth32F :
                return true;
            default :
                return false;
        }
    }

    static GLenum texture_format_to_gl_internal(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGBA8 : return GL_RGBA8;
            case TextureFormat::RGB8 : return GL_RGB8;
            case TextureFormat::RGBA16F : return GL_RGBA16F;
            case TextureFormat::RGBA32F : return GL_RGBA32F;
            case TextureFormat::R32_UINT : return GL_R32UI;
            case TextureFormat::Depth24Stencil8 : return GL_DEPTH24_STENCIL8;
            case TextureFormat::Depth32F : return GL_DEPTH_COMPONENT32F;
            default : return 0;
        }
    }

    GLFramebuffer::GLFramebuffer(const FramebufferSpecification& spec)
        : specification_(spec) {
        for (const auto& attachment_spec : specification_.attachments) {
            if (is_depth_format(attachment_spec.format)) {
                depth_attachment_spec_ = attachment_spec;
            }
            else {
                color_attachment_specs_.push_back(attachment_spec);
            }
        }
        invalidate();
    }

    GLFramebuffer::~GLFramebuffer() {
        release();
    }

    GLFramebuffer::GLFramebuffer(GLFramebuffer&& other) noexcept
        : renderer_id_(std::exchange(other.renderer_id_, 0)),
          specification_(other.specification_),
          color_attachment_specs_(std::move(other.color_attachment_specs_)),
          depth_attachment_spec_(other.depth_attachment_spec_),
          color_attachments_(std::move(other.color_attachments_)),
          depth_attachment_(std::exchange(other.depth_attachment_, 0)) {}

    GLFramebuffer& GLFramebuffer::operator=(GLFramebuffer&& other) noexcept {
        if (this != &other) {
            release();

            renderer_id_ = std::exchange(other.renderer_id_, 0);
            specification_ = other.specification_;
            color_attachment_specs_ = std::move(other.color_attachment_specs_);
            depth_attachment_spec_ = other.depth_attachment_spec_;
            color_attachments_ = std::move(other.color_attachments_);
            depth_attachment_ = std::exchange(other.depth_attachment_, 0);
        }
        return *this;
    }

    void GLFramebuffer::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_);
    }

    void GLFramebuffer::unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GLFramebuffer::resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0 || (specification_.width == width && specification_.height == height)) {
            return;
        }

        specification_.width = width;
        specification_.height = height;
        invalidate();
    }

    uint32_t GLFramebuffer::color_attachment_renderer_id(uint32_t index) const {
        if (index >= color_attachments_.size()) {
            CORE_ERROR("[GLFramebuffer] Color attachment index {0} out of bounds!", index);
            return 0;
        }
        return color_attachments_[index];
    }

    void GLFramebuffer::invalidate() {
        release();

        glCreateFramebuffers(1, &renderer_id_);

        const bool multisampled = specification_.samples > 1;

        if (!color_attachment_specs_.empty()) {
            color_attachments_.resize(color_attachment_specs_.size());
            const GLenum texture_target = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
            glCreateTextures(texture_target, static_cast<GLsizei>(color_attachments_.size()), color_attachments_.data());

            for (size_t i = 0; i < color_attachments_.size(); ++i) {
                const GLenum internal_format = texture_format_to_gl_internal(color_attachment_specs_[i].format);

                if (multisampled) {
                    glTextureStorage2DMultisample(color_attachments_[i], static_cast<GLsizei>(specification_.samples),
                                                  internal_format, static_cast<GLsizei>(specification_.width), static_cast<GLsizei>(specification_.height), GL_TRUE);
                }
                else {
                    glTextureStorage2D(color_attachments_[i], 1, internal_format,
                                       static_cast<GLsizei>(specification_.width), static_cast<GLsizei>(specification_.height));

                    glTextureParameteri(color_attachments_[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTextureParameteri(color_attachments_[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTextureParameteri(color_attachments_[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(color_attachments_[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }

                glNamedFramebufferTexture(renderer_id_, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), color_attachments_[i], 0);
            }
        }

        if (depth_attachment_spec_.format != TextureFormat::None) {
            const GLenum texture_target = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
            glCreateTextures(texture_target, 1, &depth_attachment_);

            const GLenum internal_format = texture_format_to_gl_internal(depth_attachment_spec_.format);

            if (multisampled) {
                glTextureStorage2DMultisample(depth_attachment_, static_cast<GLsizei>(specification_.samples),
                                              internal_format, static_cast<GLsizei>(specification_.width), static_cast<GLsizei>(specification_.height), GL_TRUE);
            }
            else {
                glTextureStorage2D(depth_attachment_, 1, internal_format,
                                   static_cast<GLsizei>(specification_.width), static_cast<GLsizei>(specification_.height));

                glTextureParameteri(depth_attachment_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTextureParameteri(depth_attachment_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTextureParameteri(depth_attachment_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTextureParameteri(depth_attachment_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                constexpr float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
                glTextureParameterfv(depth_attachment_, GL_TEXTURE_BORDER_COLOR, border_color);
            }

            const GLenum attachment_type = (depth_attachment_spec_.format == TextureFormat::Depth24Stencil8)
                                               ? GL_DEPTH_STENCIL_ATTACHMENT
                                               : GL_DEPTH_ATTACHMENT;

            glNamedFramebufferTexture(renderer_id_, attachment_type, depth_attachment_, 0);
        }

        if (!color_attachments_.empty()) {
            std::vector<GLenum> draw_buffers(color_attachments_.size());
            for (size_t i = 0; i < draw_buffers.size(); ++i) {
                draw_buffers[i] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i);
            }
            glNamedFramebufferDrawBuffers(renderer_id_, static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data());
        }
        else {
            // Depth-Only Pass (e.g. Cascaded Shadow Maps / Depth Prepass)
            glNamedFramebufferDrawBuffers(renderer_id_, 0, nullptr);
            glNamedFramebufferReadBuffer(renderer_id_, GL_NONE);
        }

        if (glCheckNamedFramebufferStatus(renderer_id_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            CORE_ERROR("[GLFramebuffer] Framebuffer is incomplete!");
        }
    }

    void GLFramebuffer::release() {
        if (renderer_id_ != 0) {
            glDeleteFramebuffers(1, &renderer_id_);
            glDeleteTextures(static_cast<GLsizei>(color_attachments_.size()), color_attachments_.data());
            if (depth_attachment_ != 0) {
                glDeleteTextures(1, &depth_attachment_);
            }

            renderer_id_ = 0;
            color_attachments_.clear();
            depth_attachment_ = 0;
        }
    }
} // engine
