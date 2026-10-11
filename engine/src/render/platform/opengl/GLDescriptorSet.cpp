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
        auto it = std::ranges::find_if(ubo_bindings_,
                                       [binding_slot](const UniformBufferBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != ubo_bindings_.end()) {
            it->buffer = &buffer;
            it->offset = offset;
            it->range = range;
        }
        else {
            ubo_bindings_.push_back({binding_slot, &buffer, offset, range});
        }
    }

    void GLDescriptorSet::write_storage_buffer(uint32_t binding_slot, const GLShaderStorageBuffer& buffer, size_t offset, size_t range) {
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
        auto it = std::ranges::find_if(texture_bindings_,
                                       [binding_slot](const TextureBinding& binding) { return binding.binding_slot == binding_slot; });

        if (it != texture_bindings_.end()) {
            it->texture = &texture;
        }
        else {
            texture_bindings_.push_back({binding_slot, &texture});
        }
    }

    void GLDescriptorSet::bind([[maybe_unused]] DescriptorSetIndex set_index) const {
        // Hinweis: `set_index` kann für Vulkan/DX12-Offsetting genutzt werden.
        // In OpenGL mappen wir direkt auf die jeweiligen Binding Slots:

        for (const auto& [binding_slot, buffer, offset, range] : ubo_bindings_) {
            if (!buffer) continue;

            if (range == 0) {
                glBindBufferBase(GL_UNIFORM_BUFFER, binding_slot, buffer->renderer_id());
            }
            else {
                glBindBufferRange(GL_UNIFORM_BUFFER, binding_slot, buffer->renderer_id(),
                                  static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(range));
            }
        }

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

        for (const auto& [binding_slot, texture] : texture_bindings_) {
            if (!texture) continue;
            glBindTextureUnit(binding_slot, texture->renderer_id());
        }
    }
} // engine
