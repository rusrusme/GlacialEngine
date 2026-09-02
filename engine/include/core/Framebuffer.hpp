//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine {
    class Framebuffer {
    public:
        Framebuffer(const uint32_t width, const uint32_t height) { resize(width, height); }
        ~Framebuffer() { invalidate(); }

        void bind() const;

        void unbind() const;

        void resize(uint32_t width, uint32_t height);

        [[nodiscard]] uint32_t get_color_attachment_id() const { return color_attachment_id_; }
        [[nodiscard]] uint32_t get_width() const { return width_; }
        [[nodiscard]] uint32_t get_height() const { return height_; }
        uint32_t get_depth_attachment_id() const;

    private:
        void invalidate() const;

        uint32_t fbo_id_ = 0;
        uint32_t color_attachment_id_ = 0;
        uint32_t depth_attachment_id_ = 0;
        uint32_t width_ = 1280, height_ = 720;
    };
}
