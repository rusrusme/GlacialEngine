//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/PostProcessor.hpp"

namespace engine {
    PostProcessor::~PostProcessor() {
        shutdown();
    }

    void PostProcessor::init(const uint32_t width, const uint32_t height) {
        width_ = width;
        height_ = height;

        glCreateVertexArrays(1, &screen_vao_);

        composite_shader_ = Shader::from_file(
            "assets/shaders/engine/postprocess/fullscreen.vert",
            "assets/shaders/engine/postprocess/composite.frag"
        );

        create_buffers(width_, height_);
    }

    void PostProcessor::shutdown() {
        destroy_buffers();
        if (screen_vao_ != 0) {
            glDeleteVertexArrays(1, &screen_vao_);
            screen_vao_ = 0;
        }
        composite_shader_.reset();
    }

    void PostProcessor::resize(const uint32_t width, const uint32_t height) {
        if (width == 0 || height == 0) return;
        if (width_ == width && height_ == height) return;

        width_ = width;
        height_ = height;

        destroy_buffers();
        create_buffers(width_, height_);
    }

    void PostProcessor::begin_scene() {
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo_);
        glViewport(0, 0, width_, height_);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void PostProcessor::end_scene() {
        // Bind internal final LDR target
        glBindFramebuffer(GL_FRAMEBUFFER, final_fbo_);
        glViewport(0, 0, width_, height_);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!composite_shader_) return;

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);

        composite_shader_->bind();

        // Bind HDR Color Attachment to Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdr_color_tex_);
        composite_shader_->set_int("u_hdr_scene", 0);

        // Pass Post-Processing settings uniforms
        composite_shader_->set_float("u_exposure", settings_.exposure);
        composite_shader_->set_float("u_gamma", settings_.gamma);
        composite_shader_->set_int("u_tone_mapper", static_cast<int>(settings_.tone_mapper));

        // Vignette
        composite_shader_->set_int("u_enable_vignette", settings_.enable_vignette ? 1 : 0);
        composite_shader_->set_float("u_vignette_radius", settings_.vignette_radius);
        composite_shader_->set_float("u_vignette_softness", settings_.vignette_softness);

        // Chromatic Aberration
        composite_shader_->set_int("u_enable_chromatic_aberration", settings_.enable_chromatic_aberration ? 1 : 0);
        composite_shader_->set_float("u_chromatic_strength", settings_.chromatic_aberration_strength);

        // Render full-screen triangle using zero-buffer gl_VertexID trick
        glBindVertexArray(screen_vao_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        composite_shader_->unbind();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width_, height_);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
    }

    void PostProcessor::render_to_screen() {
        // Blit final_fbo_ color attachment straight to swapchain (FBO 0)
        glBlitNamedFramebuffer(
            final_fbo_, 0,
            0, 0, width_, height_,
            0, 0, width_, height_,
            GL_COLOR_BUFFER_BIT, GL_NEAREST
        );
    }

    void PostProcessor::create_buffers(uint32_t width, uint32_t height) {
        // HDR Input Buffer (RGBA16F + Depth)
        glCreateFramebuffers(1, &hdr_fbo_);
        glCreateTextures(GL_TEXTURE_2D, 1, &hdr_color_tex_);
        glTextureStorage2D(hdr_color_tex_, 1, GL_RGBA16F, width, height);
        glTextureParameteri(hdr_color_tex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(hdr_color_tex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glNamedFramebufferTexture(hdr_fbo_, GL_COLOR_ATTACHMENT0, hdr_color_tex_, 0);

        glCreateRenderbuffers(1, &hdr_depth_rbo_);
        glNamedRenderbufferStorage(hdr_depth_rbo_, GL_DEPTH24_STENCIL8, width, height);
        glNamedFramebufferRenderbuffer(hdr_fbo_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, hdr_depth_rbo_);

        // Final LDR Buffer (RGBA8 Output)
        glCreateFramebuffers(1, &final_fbo_);
        glCreateTextures(GL_TEXTURE_2D, 1, &final_color_tex_);
        glTextureStorage2D(final_color_tex_, 1, GL_RGBA8, width, height);
        glTextureParameteri(final_color_tex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(final_color_tex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glNamedFramebufferTexture(final_fbo_, GL_COLOR_ATTACHMENT0, final_color_tex_, 0);
    }

    void PostProcessor::destroy_buffers() {
        if (hdr_fbo_ != 0) {
            glDeleteFramebuffers(1, &hdr_fbo_);
            glDeleteTextures(1, &hdr_color_tex_);
            glDeleteRenderbuffers(1, &hdr_depth_rbo_);
            hdr_fbo_ = 0;
        }
        if (final_fbo_ != 0) {
            glDeleteFramebuffers(1, &final_fbo_);
            glDeleteTextures(1, &final_color_tex_);
            final_fbo_ = 0;
        }
    }
}
