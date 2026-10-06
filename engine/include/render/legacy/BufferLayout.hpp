//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace engine {
    enum class ShaderDataType {
        None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };

    constexpr uint32_t shader_data_type_size(ShaderDataType type) {
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
            default : return 0;
        }
    }

    struct BufferElement {
        std::string name;
        ShaderDataType type;
        uint32_t size;
        size_t offset = 0;
        bool normalized;

        BufferElement(ShaderDataType type, std::string name, bool normalized = false);

        [[nodiscard]] uint32_t get_component_count() const;
    };

    class BufferLayout {
    public:
        BufferLayout() = default;
        BufferLayout(std::initializer_list<BufferElement> elements);

        [[nodiscard]] uint32_t get_stride() const { return stride_; }
        [[nodiscard]] const std::vector<BufferElement>& get_elements() const { return elements_; }

        [[nodiscard]] auto begin() const { return elements_.begin(); }
        [[nodiscard]] auto end() const { return elements_.end(); }

    private:
        void calculate_offsets_and_stride();

        std::vector<BufferElement> elements_;
        uint32_t stride_ = 0;
    };
}
