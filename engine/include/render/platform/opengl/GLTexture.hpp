//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#include <cstdint>
#include <string>
#include <glad/glad.h>

namespace engine {
    class GLTexture2D {
    public:
        GLTexture2D() = default;
        explicit GLTexture2D(const TextureSpecification& spec);
        GLTexture2D(const TextureSpecification& spec, const void* data, size_t size);
        explicit GLTexture2D(const std::string& filepath);
        ~GLTexture2D();

        GLTexture2D(const GLTexture2D&) = delete;
        GLTexture2D& operator=(const GLTexture2D&) = delete;

        GLTexture2D(GLTexture2D&& other) noexcept;
        GLTexture2D& operator=(GLTexture2D&& other) noexcept;

        void set_data(const void* data, size_t size);
        void bind(uint32_t slot = 0) const;

        [[nodiscard]] uint32_t width() const { return specification_.width; }
        [[nodiscard]] uint32_t height() const { return specification_.height; }

        [[nodiscard]] const TextureSpecification& specification() const { return specification_; }
        [[nodiscard]] TextureFormat format() const { return specification_.format; }

        [[nodiscard]] uint32_t renderer_id() const { return renderer_id_; }

        [[nodiscard]] uint32_t mip_levels() const { return mip_levels_; }
        [[nodiscard]] GLenum internal_format() const { return internal_format_; }

    private:
        void create_gpu_resources();

        uint32_t renderer_id_ = 0;
        TextureSpecification specification_{};
        GLenum internal_format_ = 0;
        GLenum data_format_ = 0;
        GLenum data_type_ = 0;

        uint32_t mip_levels_ = 1;
    };
} // namespace engine
