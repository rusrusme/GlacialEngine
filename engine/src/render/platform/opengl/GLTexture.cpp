//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLTexture.hpp"
#include "core/Log.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>

#include <stb_image.h>

namespace engine {
    static void texture_format_to_gl(TextureFormat format, GLenum& internal_format, GLenum& data_format, GLenum& data_type) {
        switch (format) {
            case TextureFormat::RGBA8 :
                internal_format = GL_RGBA8;
                data_format = GL_RGBA;
                data_type = GL_UNSIGNED_BYTE;
                break;
            case TextureFormat::RGB8 :
                internal_format = GL_RGB8;
                data_format = GL_RGB;
                data_type = GL_UNSIGNED_BYTE;
                break;
            case TextureFormat::RGBA16F :
                internal_format = GL_RGBA16F;
                data_format = GL_RGBA;
                data_type = GL_FLOAT;
                break;
            case TextureFormat::RGBA32F :
                internal_format = GL_RGBA32F;
                data_format = GL_RGBA;
                data_type = GL_FLOAT;
                break;
            case TextureFormat::R32_UINT :
                internal_format = GL_R32UI;
                data_format = GL_RED_INTEGER;
                data_type = GL_UNSIGNED_INT;
                break;
            case TextureFormat::Depth24Stencil8 :
                internal_format = GL_DEPTH24_STENCIL8;
                data_format = GL_DEPTH_STENCIL;
                data_type = GL_UNSIGNED_INT_24_8;
                break;
            case TextureFormat::Depth32F :
                internal_format = GL_DEPTH_COMPONENT32F;
                data_format = GL_DEPTH_COMPONENT;
                data_type = GL_FLOAT;
                break;
            case TextureFormat::R32F :
                internal_format = GL_R32F;
                data_format = GL_RED;
                data_type = GL_FLOAT;
                break;
            case TextureFormat::None :
            default :
                CORE_ERROR("[GLTexture] Unsupported TextureFormat!");
                internal_format = 0;
                data_format = 0;
                data_type = 0;
                break;
        }
    }

    static GLenum texture_wrap_to_gl(TextureWrap wrap) {
        switch (wrap) {
            case TextureWrap::Repeat : return GL_REPEAT;
            case TextureWrap::ClampToEdge : return GL_CLAMP_TO_EDGE;
            case TextureWrap::MirroredRepeat : return GL_MIRRORED_REPEAT;
            case TextureWrap::Border : return GL_CLAMP_TO_BORDER;
        }
        return GL_REPEAT;
    }

    GLTexture2D::GLTexture2D(const TextureSpecification& spec)
        : specification_(spec) {
        texture_format_to_gl(specification_.format, internal_format_, data_format_, data_type_);
        create_gpu_resources();
    }

    GLTexture2D::GLTexture2D(const TextureSpecification& spec, const void* data, size_t size)
        : GLTexture2D(spec) {
        if (data && size > 0) {
            set_data(data, size);
        }
    }

    GLTexture2D::GLTexture2D(const std::string& filepath) {
        if (!std::filesystem::exists(filepath)) {
            CORE_ERROR("[GLTexture] Image file not found: '{0}'", filepath);
            return;
        }

        int width = 0;
        int height = 0;
        int channels = 0;

        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

        if (!data) {
            CORE_ERROR("[GLTexture] Failed to load image: '{0}'", filepath);
            return;
        }

        specification_.width = static_cast<uint32_t>(width);
        specification_.height = static_cast<uint32_t>(height);

        if (channels == 4) {
            specification_.format = TextureFormat::RGBA8;
        }
        else if (channels == 3) {
            specification_.format = TextureFormat::RGB8;
        }
        else {
            CORE_WARN("[GLTexture] Image '{0}' has {1} channels. Defaulting to RGBA8.", filepath, channels);
            specification_.format = TextureFormat::RGBA8;
        }

        texture_format_to_gl(specification_.format, internal_format_, data_format_, data_type_);
        create_gpu_resources();

        const size_t size = static_cast<size_t>(width * height * channels);
        set_data(data, size);

        stbi_image_free(data);
    }

    GLTexture2D::~GLTexture2D() {
        if (renderer_id_ != 0) {
            glDeleteTextures(1, &renderer_id_);
        }
    }

    GLTexture2D::GLTexture2D(GLTexture2D&& other) noexcept
        : renderer_id_(std::exchange(other.renderer_id_, 0)),
          specification_(other.specification_),
          internal_format_(other.internal_format_),
          data_format_(other.data_format_),
          data_type_(other.data_type_) {}

    GLTexture2D& GLTexture2D::operator=(GLTexture2D&& other) noexcept {
        if (this != &other) {
            if (renderer_id_ != 0) {
                glDeleteTextures(1, &renderer_id_);
            }
            renderer_id_ = std::exchange(other.renderer_id_, 0);
            specification_ = other.specification_;
            internal_format_ = other.internal_format_;
            data_format_ = other.data_format_;
            data_type_ = other.data_type_;
        }
        return *this;
    }

    void GLTexture2D::set_data(const void* data, size_t size) {
        glTextureSubImage2D(renderer_id_, 0, 0, 0,
                            static_cast<GLsizei>(specification_.width),
                            static_cast<GLsizei>(specification_.height),
                            data_format_, data_type_, data);

        if (specification_.generate_mipmaps) {
            glGenerateTextureMipmap(renderer_id_);
        }
    }

    void GLTexture2D::bind(uint32_t slot) const {
        glBindTextureUnit(slot, renderer_id_);
    }

    void GLTexture2D::create_gpu_resources() {
        glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id_);

        mip_levels_ = 1;
        if (specification_.generate_mipmaps) {
            mip_levels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(specification_.width, specification_.height)))) + 1;
        }

        glTextureStorage2D(renderer_id_, static_cast<GLsizei>(mip_levels_), internal_format_,
                           static_cast<GLsizei>(specification_.width), static_cast<GLsizei>(specification_.height));

        GLenum min_filter = GL_LINEAR;
        if (specification_.min_filter == TextureFilter::Nearest) {
            min_filter = specification_.generate_mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
        }
        else {
            min_filter = specification_.generate_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        }

        GLenum mag_filter = (specification_.mag_filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR;

        glTextureParameteri(renderer_id_, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(min_filter));
        glTextureParameteri(renderer_id_, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(mag_filter));
        glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_S, static_cast<GLint>(texture_wrap_to_gl(specification_.wrap_s)));
        glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_T, static_cast<GLint>(texture_wrap_to_gl(specification_.wrap_t)));
    }
} // namespace engine
