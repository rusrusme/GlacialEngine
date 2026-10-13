//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#include <cstdint>
#include <utility>

namespace engine {
    // Descriptor binding kinds. This mirrors the Vulkan/DX12 descriptor model;
    // the OpenGL backend maps these onto its binding points (see GLDescriptorSet).
    enum class DescriptorType : uint32_t {
        UniformBuffer  = 0,
        StorageBuffer  = 1,
        SampledTexture = 2,
        StorageImage   = 3,
        Sampler        = 4
    };

    constexpr const char* to_string(const DescriptorType type) {
        switch (type) {
            case DescriptorType::UniformBuffer : return "UniformBuffer";
            case DescriptorType::StorageBuffer : return "StorageBuffer";
            case DescriptorType::SampledTexture : return "SampledTexture";
            case DescriptorType::StorageImage : return "StorageImage";
            case DescriptorType::Sampler : return "Sampler";
        }
        return "Unknown";
    }

    // Shader stage visibility of a binding. OpenGL 4.6 has no per-binding stage
    // masks (visibility follows the shader interface), so the backend only
    // validates against it; Vulkan/DX12 use it directly.
    enum class ShaderStage : uint32_t {
        None        = 0,
        Vertex      = 1 << 0,
        Fragment    = 1 << 1,
        Compute     = 1 << 2,
        AllGraphics = Vertex | Fragment,
        All         = Vertex | Fragment | Compute
    };

    constexpr ShaderStage operator|(const ShaderStage a, const ShaderStage b) {
        return static_cast<ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr ShaderStage operator&(const ShaderStage a, const ShaderStage b) {
        return static_cast<ShaderStage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    struct DescriptorSetLayoutBinding {
        uint32_t binding{0};
        DescriptorType type{DescriptorType::UniformBuffer};
        uint32_t count{1};                             // > 1 declares a descriptor array (bindless)
        ShaderStage stage{ShaderStage::All};
        bool bindless{false};
    };

    // Declares the contents of a single descriptor set. This is the contract a
    // pipeline layout is built from, and what GLDescriptorSet validates its
    // writes against. Vulkan builds a VkDescriptorSetLayout from it and DX12 a
    // root signature — neither can be created without it.
    struct DescriptorSetLayoutDesc {
        Vector<DescriptorSetLayoutBinding> bindings{};
        bool bindless{false};

        [[nodiscard]] bool empty() const { return bindings.empty(); }

        [[nodiscard]] const DescriptorSetLayoutBinding* find(const uint32_t binding_slot) const {
            for (const DescriptorSetLayoutBinding& binding : bindings) {
                if (binding.binding == binding_slot) {
                    return &binding;
                }
            }
            return nullptr;
        }

        [[nodiscard]] uint32_t binding_count() const {
            return static_cast<uint32_t>(bindings.size());
        }
    };

    struct PushConstantRange {
        ShaderStage stage{ShaderStage::All};
        uint32_t offset{0};
        uint32_t size{0};
    };

    struct DescriptorSetLayoutEntry {
        DescriptorSetIndex set{DescriptorSetIndex::Frame};
        DescriptorSetLayoutDesc layout{};
    };

    // A pipeline's complete binding contract: one layout per descriptor set
    // index, plus push constant ranges. Vulkan (VkPipelineLayout) and DX12
    // (root signature) are both created from this.
    struct PipelineLayoutDesc {
        Vector<DescriptorSetLayoutEntry> sets{};
        Vector<PushConstantRange> push_constants{};

        [[nodiscard]] bool empty() const { return sets.empty() && push_constants.empty(); }

        [[nodiscard]] const DescriptorSetLayoutDesc* set_layout(const DescriptorSetIndex set) const {
            for (const DescriptorSetLayoutEntry& entry : sets) {
                if (entry.set == set) {
                    return &entry.layout;
                }
            }
            return nullptr;
        }
    };

    // Value wrapper so pipelines and descriptor sets can carry a layout by name
    // without the RHI depending on a backend object.
    class PipelineLayout {
    public:
        PipelineLayout() = default;
        explicit PipelineLayout(PipelineLayoutDesc desc) : desc_(std::move(desc)) {}

        [[nodiscard]] const PipelineLayoutDesc& description() const noexcept { return desc_; }
        [[nodiscard]] bool empty() const noexcept { return desc_.empty(); }

        [[nodiscard]] const DescriptorSetLayoutDesc* set_layout(const DescriptorSetIndex set) const {
            return desc_.set_layout(set);
        }

    private:
        PipelineLayoutDesc desc_{};
    };
}
