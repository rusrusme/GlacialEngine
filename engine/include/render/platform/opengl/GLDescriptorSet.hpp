//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/DescriptorSetLayout.hpp"
#include "render/rhi/RHITypes.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace engine {
    class GLIndirectBuffer;
    class GLUniformBuffer;
    class GLShaderStorageBuffer;
    class GLTexture2D;

    // Bindings store the GL object name and the metadata needed at bind time.
    // Holding the resource by pointer would dangle as soon as the buffer or texture
    // that was written in is destroyed or reallocated (e.g. when a system grows its
    // storage buffer and swaps in a new one).
    struct UniformBufferBinding {
        uint32_t binding_slot{0};
        uint32_t buffer_id{0};
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
        uint32_t texture_id{0};
    };

    enum class ImageAccess { ReadOnly, WriteOnly, ReadWrite };

    struct ImageBinding {
        uint32_t binding_slot{0};
        uint32_t texture_id{0};
        uint32_t internal_format{0}; // cached so bind() never touches the texture object
        uint32_t mip_level{0};
        ImageAccess access{ImageAccess::WriteOnly};
    };

    class GLDescriptorSet {
    public:
        GLDescriptorSet() = default;
        explicit GLDescriptorSet(DescriptorSetLayoutDesc layout);
        ~GLDescriptorSet() = default;

        GLDescriptorSet(const GLDescriptorSet&) = delete;
        GLDescriptorSet& operator=(const GLDescriptorSet&) = delete;
        GLDescriptorSet(GLDescriptorSet&&) noexcept = default;
        GLDescriptorSet& operator=(GLDescriptorSet&&) noexcept = default;

        void set_layout(DescriptorSetLayoutDesc layout) { layout_ = std::move(layout); }
        [[nodiscard]] const DescriptorSetLayoutDesc& layout() const noexcept { return layout_; }

        void write_uniform_buffer(uint32_t binding_slot, const GLUniformBuffer& buffer, size_t offset = 0, size_t range = 0);
        void write_storage_buffer(uint32_t binding_slot, const GLShaderStorageBuffer& buffer, size_t offset = 0, size_t range = 0);
        void write_storage_buffer(uint32_t binding_slot, const GLIndirectBuffer& buffer, size_t offset = 0, size_t range = 0);
        void write_texture(uint32_t binding_slot, const GLTexture2D& texture);
        void write_storage_image(uint32_t binding_slot, const GLTexture2D& texture, uint32_t mip_level = 0, ImageAccess access = ImageAccess::WriteOnly);

        /// No-Op in OpenGL (For Vulkan/DX12)
        void update() noexcept {}

        void bind(DescriptorSetIndex set_index) const;

    private:
        // Rejects a write whose slot or type is not declared by the layout. Always
        // accepts while the set has no layout, which keeps the previous behaviour
        // for callers that bind by raw slot number.
        [[nodiscard]] bool validate_binding(uint32_t binding_slot, DescriptorType type) const;

        DescriptorSetLayoutDesc layout_{};

        Vector<UniformBufferBinding> ubo_bindings_;
        Vector<StorageBufferBinding> ssbo_bindings_;
        Vector<TextureBinding> texture_bindings_;
        Vector<ImageBinding> image_bindings_;
    };

} // namespace engine
