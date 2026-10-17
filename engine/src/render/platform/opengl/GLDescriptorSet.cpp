//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLDescriptorSet.hpp"

#include <algorithm>
#include <glad/glad.h>

#include "core/Log.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    void GLDescriptorSet::write_uniform_buffer(uint32_t binding_slot, const GLUniformBuffer& buffer, size_t offset, size_t range) {
        const auto it = std::ranges::find_if(ubo_bindings_,
                                             [binding_slot](const UniformBufferBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != ubo_bindings_.end()) {
            it->buffer = buffer.renderer_id();
            it->offset = offset;
            it->range = range;
        }
        else {
            ubo_bindings_.push_back({
                .binding_slot = binding_slot,
                .buffer       = buffer.renderer_id(),
                .offset       = offset,
                .range        = range
            });
        }
    }

    void GLDescriptorSet::write_storage_buffer(uint32_t binding_slot, const GLShaderStorageBuffer& buffer, size_t offset, size_t range) {
        const auto it = std::ranges::find_if(ssbo_bindings_,
                                             [binding_slot](const StorageBufferBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != ssbo_bindings_.end()) {
            it->buffer_id = buffer.renderer_id();
            it->offset = offset;
            it->range = range;
        }
        else {
            ssbo_bindings_.push_back({
                .binding_slot = binding_slot,
                .buffer_id    = buffer.renderer_id(),
                .offset       = offset,
                .range        = range
            });
        }
    }

    void GLDescriptorSet::write_storage_buffer(uint32_t binding_slot, const GLIndirectBuffer& buffer, size_t offset, size_t range) {
        const auto it = std::ranges::find_if(ssbo_bindings_,
                                             [binding_slot](const StorageBufferBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != ssbo_bindings_.end()) {
            it->buffer_id = buffer.renderer_id();
            it->offset = offset;
            it->range = range;
        }
        else {
            ssbo_bindings_.push_back({
                .binding_slot = binding_slot,
                .buffer_id    = buffer.renderer_id(),
                .offset       = offset,
                .range        = range
            });
        }
    }

    void GLDescriptorSet::write_texture(uint32_t binding_slot, const GLTexture2D& texture) {
        const auto it = std::ranges::find_if(texture_bindings_,
                                             [binding_slot](const TextureBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != texture_bindings_.end()) {
            it->texture = texture.renderer_id();
        }
        else {
            texture_bindings_.push_back({
                .binding_slot = binding_slot,
                .texture      = texture.renderer_id()
            });
        }
    }

    void GLDescriptorSet::write_storage_image(uint32_t binding_slot, const GLTexture2D& texture, uint32_t mip_level, ImageAccess access) {
        const auto it = std::ranges::find_if(image_bindings_,
                                             [binding_slot](const ImageBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != image_bindings_.end()) {
            it->texture = texture.renderer_id();
            it->mip_level = mip_level;
            it->access = access;
        }
        else {
            image_bindings_.push_back({
                .binding_slot = binding_slot,
                .texture      = texture.renderer_id(),
                .mip_level    = mip_level,
                .access       = access
            });
        }
    }

    void GLDescriptorSet::bind() const {
        // UBO
        for (const auto& [binding_slot, buffer_id, offset, range] : ubo_bindings_) {
            if (!buffer_id) continue;

            if (range == 0) {
                glBindBufferBase(GL_UNIFORM_BUFFER, binding_slot, buffer_id);
            }
            else {
                glBindBufferRange(GL_UNIFORM_BUFFER, binding_slot, buffer_id,
                                  static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(range));
            }
        }

        // SSBO
        for (const auto& [binding_slot, buffer_id, offset, range] : ssbo_bindings_) {
            if (!buffer_id) continue;

            if (range == 0) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_slot, buffer_id);
            }
            else {
                glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding_slot, buffer_id,
                                  static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(range));
            }
        }

        // Texture
        for (const auto& [binding_slot, texture_id] : texture_bindings_) {
            if (!texture_id) continue;
            glBindTextureUnit(binding_slot, texture_id);
        }

        // Image
        for (const auto& [binding_slot, texture_id, internal_format, mip_level, access] : image_bindings_) {
            if (!texture_id) continue;

            GLenum gl_access = GL_WRITE_ONLY;
            if (access == ImageAccess::ReadOnly) gl_access = GL_READ_ONLY;
            else if (access == ImageAccess::ReadWrite) gl_access = GL_READ_WRITE;

            GLenum format = internal_format;
            if (format == GL_DEPTH_COMPONENT32F) {
                format = GL_R32F;
            }
            else if (format == GL_DEPTH24_STENCIL8) {
                format = GL_R32UI;
            }

            glBindImageTexture(
                binding_slot,
                texture_id,
                static_cast<GLint>(mip_level),
                GL_FALSE,
                0,
                gl_access,
                format
            );
        }
    }
} // engine
