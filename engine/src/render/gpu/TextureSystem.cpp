//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/gpu/TextureSystem.hpp"

#include <cassert>
#include <stb_image.h>

#include "core/Log.hpp"

namespace engine {
    TextureSystem::TextureSystem(uint32_t width, uint32_t height, uint32_t max_layers)
        : max_layers_(max_layers) {
        ///////////////////////////////////////////////////////////////////////////////
        TextureSpecification spec{};
        spec.width = width;
        spec.height = height;
        spec.format = TextureFormat::RGBA8;
        spec.min_filter = TextureFilter::Linear;
        spec.mag_filter = TextureFilter::Linear;
        spec.wrap_s = TextureWrap::Repeat;
        spec.wrap_t = TextureWrap::Repeat;
        spec.generate_mipmaps = true;

        global_texture_array_ = std::make_unique<Texture2D>(spec);
    }

    uint32_t TextureSystem::register_texture(const std::string& filepath) {
        assert(current_slice_index_ < max_layers_ && "Exceeded maximum global texture array layer capacity!");

        int width = 0;
        int height = 0;
        int channels = 0;

        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!data) {
            CORE_ERROR("[TextureSystem] Failed to load texture at path: {}", filepath.c_str());
            return 0; // Fallback Default-Slice (Slice 0)
        }

        const uint32_t slice_index = current_slice_index_++;
        const size_t image_size = static_cast<size_t>(width) * height * 4;

        if (global_texture_array_) {
            global_texture_array_->set_data(data, image_size);
        }

        stbi_image_free(data);
        return slice_index;
    }

    void TextureSystem::bind(DescriptorSet& descriptor_set, uint32_t binding_slot) const {
        if (global_texture_array_) {
            descriptor_set.write_texture(binding_slot, *global_texture_array_);
        }
    }
}
