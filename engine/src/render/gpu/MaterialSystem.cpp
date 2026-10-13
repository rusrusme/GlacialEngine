//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/gpu/MaterialSystem.hpp"

#include <algorithm>
#include <cassert>
#include <memory>

namespace engine {
    MaterialSystem::MaterialSystem(size_t initial_capacity)
        : allocated_gpu_capacity_(initial_capacity) {
        materials_.reserve(initial_capacity);

        const size_t initial_bytes = allocated_gpu_capacity_ * sizeof(GPUMaterialData);
        material_ssbo_ = std::make_unique<ShaderStorageBuffer>(initial_bytes, 2);
    }

    uint32_t MaterialSystem::register_material(const GPUMaterialData& data) {
        const uint32_t material_id = static_cast<uint32_t>(materials_.size());
        materials_.push_back(data);

        is_dirty_ = true;
        return material_id;
    }

    void MaterialSystem::update_material(uint32_t material_id, const GPUMaterialData& data) {
        assert(material_id < materials_.size() && "Material ID out of bounds");

        materials_[material_id] = data;
        is_dirty_ = true;
    }

    void MaterialSystem::flush_to_gpu() {
        if (!is_dirty_ || materials_.empty()) {
            return;
        }

        if (materials_.size() > allocated_gpu_capacity_) {
            grow_gpu_buffer(materials_.size());
        }

        const size_t upload_bytes = materials_.size() * sizeof(GPUMaterialData);
        material_ssbo_->set_data(materials_.data(), upload_bytes, 0);

        is_dirty_ = false;
    }

    void MaterialSystem::bind(DescriptorSet& descriptor_set, uint32_t binding_slot) const {
        if (material_ssbo_) {
            descriptor_set.write_storage_buffer(binding_slot, *material_ssbo_);
        }
    }

    const GPUMaterialData& MaterialSystem::material(uint32_t material_id) const {
        assert(material_id < materials_.size() && "Material ID out of bounds");
        return materials_[material_id];
    }

    void MaterialSystem::grow_gpu_buffer(size_t required_capacity) {
        size_t new_capacity = allocated_gpu_capacity_ > 0 ? allocated_gpu_capacity_ * 2 : 256;
        while (new_capacity < required_capacity) {
            new_capacity *= 2;
        }

        allocated_gpu_capacity_ = new_capacity;
        const size_t new_bytes = allocated_gpu_capacity_ * sizeof(GPUMaterialData);

        material_ssbo_ = std::make_unique<ShaderStorageBuffer>(new_bytes, 2);
    }
}
