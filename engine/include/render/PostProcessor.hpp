//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "render/Shader.hpp"

namespace engine {
    enum class ToneMapAlgorithm {
        None = -1,
        ACESFilmic = 0,
        Reinhard,
        Uncharted2
    };

    struct PostProcessSettings {
        float exposure = 1.0f;
        float gamma = 2.2f;

        // Vignette
        bool enable_vignette = false;
        float vignette_radius = 0.8f;
        float vignette_softness = 0.45f;

        // Chromatic Aberration
        bool enable_chromatic_aberration = false;
        float chromatic_aberration_strength = 0.003f;

        ToneMapAlgorithm tone_mapper = ToneMapAlgorithm::None;
    };

    class PostProcessor {
    public:
        PostProcessor() = default;
        ~PostProcessor();

        PostProcessor(const PostProcessor&) = delete;
        PostProcessor& operator=(const PostProcessor&) = delete;

        void init(uint32_t width, uint32_t height);
        void shutdown();
        void resize(uint32_t width, uint32_t height);

        /** @brief Binds the internal floating-point HDR framebuffer to capture the scene */
        void begin_scene();

        /** @brief Unbinds HDR target, executes screen-space shader chain, and draws to target_fbo (0 = Screen) */
        void end_scene();

        /** @brief Helper to blit/render the final LDR texture to the screen swapchain (FBO 0) */
        void render_to_screen();

        [[nodiscard]] uint32_t get_hdr_framebuffer_id() const { return hdr_fbo_; }
        [[nodiscard]] uint32_t get_hdr_color_texture_id() const { return hdr_color_tex_; }
        [[nodiscard]] uint32_t get_final_color_texture() const { return final_color_tex_; }

        PostProcessSettings& get_settings() { return settings_; }
        [[nodiscard]] const PostProcessSettings& get_settings() const { return settings_; }

    private:
        void create_buffers(uint32_t width, uint32_t height);
        void destroy_buffers();

        uint32_t width_ = 1280;
        uint32_t height_ = 720;

        // Primary HDR Render Target (16-bit float channels)
        uint32_t hdr_fbo_ = 0;
        uint32_t hdr_color_tex_ = 0;
        uint32_t hdr_depth_rbo_ = 0;

        // Ping-Pong Framebuffers for multi-pass blurs / passes
        uint32_t final_fbo_ = 0;
        uint32_t final_color_tex_ = 0;

        // Procedural Fullscreen Triangle VAO
        uint32_t screen_vao_ = 0;

        std::unique_ptr<Shader> composite_shader_;
        PostProcessSettings settings_;
    };
}
