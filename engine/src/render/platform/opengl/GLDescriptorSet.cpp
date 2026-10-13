//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLDescriptorSet.hpp"

#include <algorithm>
#include <utility>
#include <glad/glad.h>

#include "core/Log.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    namespace {
        // A destroyed GL object's name goes back to the driver's pool, so these
        // checks catch the common "bound something that is already gone" mistake
        // rather than every stale-name case. They are a debug aid, not a substitute
        // for the owner re-writing the descriptor set when it reallocates a
        // resource (see GLDescriptorSet.hpp).
        bool buffer_is_alive([[maybe_unused]] const uint32_t binding_slot, [[maybe_unused]] const uint32_t buffer_id) {
#ifndef NDEBUG
            if (glIsBuffer(buffer_id) != GL_TRUE) {
                CORE_ERROR("[GLDescriptorSet] Binding slot {} references buffer {} which no longer exists",
                           binding_slot, buffer_id);
                return false;
            }
#endif
            return true;
        }

        bool texture_is_alive([[maybe_unused]] const uint32_t binding_slot, [[maybe_unused]] const uint32_t texture_id) {
#ifndef NDEBUG
            if (glIsTexture(texture_id) != GL_TRUE) {
                CORE_ERROR("[GLDescriptorSet] Binding slot {} references texture {} which no longer exists",
                           binding_slot, texture_id);
                return false;
            }
#endif
            return true;
        }
    }

    GLDescriptorSet::GLDescriptorSet(DescriptorSetLayoutDesc layout)
        : layout_(std::move(layout)) {}

    bool GLDescriptorSet::validate_binding(const uint32_t binding_slot, const DescriptorType type) const {
        if (layout_.empty()) {
            return true;
        }

        const DescriptorSetLayoutBinding* binding = layout_.find(binding_slot);
        if (!binding) {
            CORE_ERROR("[GLDescriptorSet] Binding slot {} is not declared by the descriptor set layout", binding_slot);
            return false;
        }

        if (binding->type != type) {
            CORE_ERROR("[GLDescriptorSet] Binding slot {} is declared as {} but was written as {}",
                       binding_slot, to_string(binding->type), to_string(type));
            return false;
        }

        return true;
    }

    void GLDescriptorSet::write_uniform_buffer(uint32_t binding_slot, const GLUniformBuffer& buffer, size_t offset, size_t range) {
        if (!validate_binding(binding_slot, DescriptorType::UniformBuffer)) {
            return;
        }

        auto it = std::ranges::find_if(ubo_bindings_,
                                       [binding_slot](const UniformBufferBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != ubo_bindings_.end()) {
            it->buffer_id = buffer.renderer_id();
            it->offset = offset;
            it->range = range;
        }
        else {
            ubo_bindings_.push_back({binding_slot, buffer.renderer_id(), offset, range});
        }
    }

    void GLDescriptorSet::write_storage_buffer(uint32_t binding_slot, const GLShaderStorageBuffer& buffer, size_t offset, size_t range) {
        if (!validate_binding(binding_slot, DescriptorType::StorageBuffer)) {
            return;
        }

        auto it = std::ranges::find_if(ssbo_bindings_,
                                       [binding_slot](const StorageBufferBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != ssbo_bindings_.end()) {
            it->buffer_id = buffer.renderer_id();
            it->offset = offset;
            it->range = range;
        }
        else {
            ssbo_bindings_.push_back({binding_slot, buffer.renderer_id(), offset, range});
        }
    }

    void GLDescriptorSet::write_storage_buffer(uint32_t binding_slot, const GLIndirectBuffer& buffer, size_t offset, size_t range) {
        if (!validate_binding(binding_slot, DescriptorType::StorageBuffer)) {
            return;
        }

        auto it = std::ranges::find_if(ssbo_bindings_,
                                       [binding_slot](const StorageBufferBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != ssbo_bindings_.end()) {
            it->buffer_id = buffer.renderer_id();
            it->offset = offset;
            it->range = range;
        }
        else {
            ssbo_bindings_.push_back({binding_slot, buffer.renderer_id(), offset, range});
        }
    }

    void GLDescriptorSet::write_texture(uint32_t binding_slot, const GLTexture2D& texture) {
        if (!validate_binding(binding_slot, DescriptorType::SampledTexture)) {
            return;
        }

        auto it = std::ranges::find_if(texture_bindings_,
                                       [binding_slot](const TextureBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != texture_bindings_.end()) {
            it->texture_id = texture.renderer_id();
        }
        else {
            texture_bindings_.push_back({binding_slot, texture.renderer_id()});
        }
    }

    void GLDescriptorSet::write_storage_image(uint32_t binding_slot, const GLTexture2D& texture, uint32_t mip_level, ImageAccess access) {
        if (!validate_binding(binding_slot, DescriptorType::StorageImage)) {
            return;
        }

        auto it = std::ranges::find_if(image_bindings_,
                                       [binding_slot](const ImageBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != image_bindings_.end()) {
            it->texture_id = texture.renderer_id();
            it->internal_format = static_cast<uint32_t>(texture.internal_format());
            it->mip_level = mip_level;
            it->access = access;
        }
        else {
            image_bindings_.push_back({binding_slot, texture.renderer_id(),
                                       static_cast<uint32_t>(texture.internal_format()), mip_level, access});
        }
    }

    void GLDescriptorSet::bind([[maybe_unused]] const DescriptorSetIndex set_index) const {
        // set_index is only meaningful for Vulkan/DX12, where it selects which
        // descriptor set is being bound. OpenGL has no set concept, so each binding
        // maps straight onto its global binding point.

        for (const auto& [binding_slot, buffer_id, offset, range] : ubo_bindings_) {
            if (buffer_id == 0 || !buffer_is_alive(binding_slot, buffer_id)) {
                continue;
            }

            if (range == 0) {
                glBindBufferBase(GL_UNIFORM_BUFFER, binding_slot, buffer_id);
            }
            else {
                glBindBufferRange(GL_UNIFORM_BUFFER, binding_slot, buffer_id,
                                  static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(range));
            }
        }

        for (const auto& [binding_slot, buffer_id, offset, range] : ssbo_bindings_) {
            if (buffer_id == 0 || !buffer_is_alive(binding_slot, buffer_id)) {
                continue;
            }

            if (range == 0) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_slot, buffer_id);
            }
            else {
                glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding_slot, buffer_id,
                                  static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(range));
            }
        }

        for (const auto& [binding_slot, texture_id] : texture_bindings_) {
            if (texture_id == 0 || !texture_is_alive(binding_slot, texture_id)) {
                continue;
            }
            glBindTextureUnit(binding_slot, texture_id);
        }

        for (const auto& [binding_slot, texture_id, internal_format, mip_level, access] : image_bindings_) {
            if (texture_id == 0 || !texture_is_alive(binding_slot, texture_id)) {
                continue;
            }

            GLenum gl_access = GL_WRITE_ONLY;
            if (access == ImageAccess::ReadOnly) gl_access = GL_READ_ONLY;
            else if (access == ImageAccess::ReadWrite) gl_access = GL_READ_WRITE;

            GLenum format = static_cast<GLenum>(internal_format);
            if (format == GL_DEPTH_COMPONENT32F) {
                format = GL_R32F;
            } else if (format == GL_DEPTH24_STENCIL8) {
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
