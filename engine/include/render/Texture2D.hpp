//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace engine {
    class Texture2D {
    public:
        Texture2D(uint32_t width, uint32_t height, uint32_t channels, const uint8_t* data);
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        Texture2D(Texture2D&& other) noexcept;
        Texture2D& operator=(Texture2D&& other) noexcept;

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
