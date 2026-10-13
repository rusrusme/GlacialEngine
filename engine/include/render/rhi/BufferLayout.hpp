//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/RHITypes.hpp"

#include <initializer_list>
#include <string>
#include <vector>

namespace engine {
    // Sentinel meaning "let the backend assign the next free attribute location".
    constexpr uint32_t kInvalidAttributeLocation = 0xFFFFFFFFu;

    constexpr uint32_t shader_data_type_size(const ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float : return 4;
            case ShaderDataType::Float2 : return 4 * 2;
            case ShaderDataType::Float3 : return 4 * 3;
            case ShaderDataType::Float4 : return 4 * 4;
            case ShaderDataType::Mat3 : return 4 * 3 * 3;
            case ShaderDataType::Mat4 : return 4 * 4 * 4;
            case ShaderDataType::Int : return 4;
            case ShaderDataType::Int2 : return 4 * 2;
            case ShaderDataType::Int3 : return 4 * 3;
            case ShaderDataType::Int4 : return 4 * 4;
            case ShaderDataType::Bool : return 1;
            case ShaderDataType::None : break;
        }
        return 0;
    }

    struct BufferElement {
        std::string name;
        ShaderDataType type = ShaderDataType::None;
        uint32_t size = 0;
        size_t offset = 0;
        bool normalized = false;
        bool per_instance = false;

        // Shader attribute location this element maps to. When left at
        // kInvalidAttributeLocation the backend assigns locations in declaration
        // order (the previous behaviour). Vulkan/DX12 need the explicit value to
        // match the shader interface.
        uint32_t location = kInvalidAttributeLocation;

        // Which vertex buffer binding slot this element reads from. Element order
        // within a layout no longer implies the buffer it belongs to.
        uint32_t binding_index = 0;

        BufferElement() = default;

        BufferElement(ShaderDataType type, std::string name, bool normalized = false, bool per_instance = false,
                      uint32_t location = kInvalidAttributeLocation, uint32_t binding_index = 0)
            : name(std::move(name)), type(type), size(shader_data_type_size(type)), normalized(normalized),
              per_instance(per_instance), location(location), binding_index(binding_index) {}

        [[nodiscard]] uint32_t component_count() const {
            switch (type) {
                case ShaderDataType::Float : return 1;
                case ShaderDataType::Float2 : return 2;
                case ShaderDataType::Float3 : return 3;
                case ShaderDataType::Float4 : return 4;
                case ShaderDataType::Mat3 : return 3 * 3;
                case ShaderDataType::Mat4 : return 4 * 4;
                case ShaderDataType::Int : return 1;
                case ShaderDataType::Int2 : return 2;
                case ShaderDataType::Int3 : return 3;
                case ShaderDataType::Int4 : return 4;
                case ShaderDataType::Bool : return 1;
                case ShaderDataType::None : break;
            }
            return 0;
        }

        // Number of consecutive attribute locations the element occupies: a Mat4
        // consumes four (one per column), everything else one.
        [[nodiscard]] uint32_t location_count() const {
            switch (type) {
                case ShaderDataType::Mat3 : return 3;
                case ShaderDataType::Mat4 : return 4;
                case ShaderDataType::None : return 0;
                default : return 1;
            }
        }

        // Vertex attribute divisor for the binding: 1 advances the attribute once
        // per instance, 0 once per vertex.
        [[nodiscard]] uint32_t divisor() const { return per_instance ? 1u : 0u; }

        [[nodiscard]] bool is_matrix() const {
            return type == ShaderDataType::Mat3 || type == ShaderDataType::Mat4;
        }

        [[nodiscard]] bool is_integer() const {
            switch (type) {
                case ShaderDataType::Int :
                case ShaderDataType::Int2 :
                case ShaderDataType::Int3 :
                case ShaderDataType::Int4 :
                case ShaderDataType::Bool : return true;
                default : return false;
            }
        }
    };

    class BufferLayout {
    public:
        BufferLayout() = default;
        BufferLayout(std::initializer_list<BufferElement> elements);

        [[nodiscard]] uint32_t stride() const { return stride_; }
        [[nodiscard]] const std::vector<BufferElement>& elements() const { return elements_; }

        [[nodiscard]] Vector<BufferElement>::iterator begin() { return elements_.begin(); }
        [[nodiscard]] Vector<BufferElement>::iterator end() { return elements_.end(); }
        [[nodiscard]] Vector<BufferElement>::const_iterator begin() const { return elements_.begin(); }
        [[nodiscard]] Vector<BufferElement>::const_iterator end() const { return elements_.end(); }

    private:
        void calculate_offsets_and_stride();

        Vector<BufferElement> elements_;
        uint32_t stride_ = 0;
    };
}
