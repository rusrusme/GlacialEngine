//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <vector>

#include "render/rhi/RHITypes.hpp"

namespace engine {
    class GLIndirectBuffer;
    class GLUniformBuffer;
    class GLShaderStorageBuffer;
    class GLTexture2D;

    struct UniformBufferBinding {
        uint32_t binding_slot{0};
        const GLUniformBuffer* buffer{nullptr};
        size_t offset{0};
        size_t range{0};
    };

    struct StorageBufferBinding {
        uint32_t binding_slot{0};
        uint32_t buffer_id{0};
        size_t offset{0};
        size_t range{0};
    };

    struct TextureBinding {
        uint32_t binding_slot{0};
        const GLTexture2D* texture{nullptr};
    };

    class GLDescriptorSet {
    public:
        GLDescriptorSet() = default;
        ~GLDescriptorSet() = default;

        GLDescriptorSet(const GLDescriptorSet&) = delete;
        GLDescriptorSet& operator=(const GLDescriptorSet&) = delete;
        GLDescriptorSet(GLDescriptorSet&&) noexcept = default;
        GLDescriptorSet& operator=(GLDescriptorSet&&) noexcept = default;

        void write_uniform_buffer(uint32_t binding_slot, const GLUniformBuffer& buffer, size_t offset = 0, size_t range = 0);
        void write_storage_buffer(uint32_t binding_slot, const GLShaderStorageBuffer& buffer, size_t offset = 0, size_t range = 0);
        void write_storage_buffer(uint32_t binding_slot, const GLIndirectBuffer& buffer, size_t offset = 0, size_t range = 0);
        void write_texture(uint32_t binding_slot, const GLTexture2D& texture);

        void bind(DescriptorSetIndex set_index) const;

    private:
        std::vector<UniformBufferBinding> ubo_bindings_;
        std::vector<StorageBufferBinding> ssbo_bindings_;
        std::vector<TextureBinding> texture_bindings_;
    };

} // namespace engine