//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/Texture2D.hpp"

namespace engine {
    Texture2D::Texture2D(uint32_t width, uint32_t height, uint32_t channels, const uint8_t* data) {
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);

        GLenum internal_format = (channels == 4) ? GL_RGBA8 : GL_RGB8;
        GLenum data_format = (channels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, data_format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    Texture2D::~Texture2D() {
        if (id_ != 0) {
            glDeleteTextures(1, &id_);
        }
    }

    Texture2D::Texture2D(Texture2D&& other) noexcept
        : id_(std::exchange(other.id_, 0)), width_(other.width_), height_(other.height_) {}

    Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
        if (this != &other) {
            if (id_ != 0) glDeleteTextures(1, &id_);
            id_ = std::exchange(other.id_, 0);
            width_ = other.width_;
            height_ = other.height_;
        }
        return *this;
    }

    void Texture2D::bind(uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id_);
    }

    void Texture2D::unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
