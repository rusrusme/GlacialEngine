//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine {
    class GLTexture2D {
    public:
        GLTexture2D(uint32_t width, uint32_t height, uint32_t channels, const uint8_t* data);
        ~GLTexture2D();

        GLTexture2D(const GLTexture2D&) = delete;
        GLTexture2D& operator=(const GLTexture2D&) = delete;

        GLTexture2D(GLTexture2D&& other) noexcept;
        GLTexture2D& operator=(GLTexture2D&& other) noexcept;

        void bind(uint32_t slot = 0) const;
        void unbind() const;

        [[nodiscard]] uint32_t get_id() const { return id_; }
        [[nodiscard]] uint32_t get_width() const { return width_; }
        [[nodiscard]] uint32_t get_height() const { return height_; }

    private:
        uint32_t id_ = 0;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
    };
}
