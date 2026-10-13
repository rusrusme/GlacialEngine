//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <vector>

#include "GPUData.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    class ShaderStorageBuffer;
    class DescriptorSet;

    /**
     * @brief Host and GPU material management system for GPU-driven bindless material passes.
     */
    class MaterialSystem {
    public:
        explicit MaterialSystem(size_t initial_capacity = 256);
        ~MaterialSystem() = default;

        MaterialSystem(const MaterialSystem&) = delete;
        MaterialSystem& operator=(const MaterialSystem&) = delete;

        MaterialSystem(MaterialSystem&&) noexcept = default;
        MaterialSystem& operator=(MaterialSystem&&) noexcept = default;

        /**
         * @brief Registers a new material and returns its unique GPU material ID.
         * @param data Material parameters.
         * @return uint32_t material_id corresponding to SSBO index.
         */
        uint32_t register_material(const GPUMaterialData& data);

        /**
         * @brief Updates an existing material's parameters in host cache and marks SSBO as dirty.
         * @param material_id Target material ID.
         * @param data Updated material parameters.
         */
        void update_material(uint32_t material_id, const GPUMaterialData& data);

        /**
         * @brief Uploads modified host material data to the GPU backing SSBO.
         */
        void flush_to_gpu();

        /**
         * @brief Binds the Material SSBO to the target descriptor set slot.
         * @param descriptor_set Target RHI descriptor set.
         * @param binding_slot Target binding index (default: slot 2).
         */
        void bind(DescriptorSet& descriptor_set, uint32_t binding_slot = 2) const;

        [[nodiscard]] const GPUMaterialData& material(uint32_t material_id) const;
        [[nodiscard]] size_t material_count() const { return materials_.size(); }
        [[nodiscard]] ShaderStorageBuffer* buffer() const { return material_ssbo_.get(); }

    private:
        void grow_gpu_buffer(size_t required_capacity);

        Vector<GPUMaterialData> materials_;
        Unique<ShaderStorageBuffer> material_ssbo_{nullptr};

        bool is_dirty_{false};
        size_t allocated_gpu_capacity_{0};
    };
}
