//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>

#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/RHITypes.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    /**
     * @brief Manages global texture array slices and bindless texture handle allocations.
     */
    class TextureSystem {
    public:
        explicit TextureSystem(uint32_t width = 2048, uint32_t height = 2048, uint32_t max_layers = 256);
        ~TextureSystem() = default;

        /**
         * @brief Loads a texture from disk and copies it into a unique slice in the global texture array.
         * @return uint32_t Texture slice index to store in GPUMaterialData.
         */
        uint32_t register_texture(const std::string& filepath);

        /**
         * @brief Binds the global texture array sampler to the descriptor set slot (default: slot 3).
         */
        void bind(DescriptorSet& descriptor_set, uint32_t binding_slot = 3) const;

    private:
        Unique<Texture2D> global_texture_array_{nullptr};
        uint32_t current_slice_index_{0};
        uint32_t max_layers_{256};
    };
}
