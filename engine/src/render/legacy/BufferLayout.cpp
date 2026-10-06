//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/BufferLayout.hpp"

namespace engine {
    BufferElement::BufferElement(ShaderDataType type, std::string name, bool normalized) : name(std::move(name)), type(type), size(shader_data_type_size(type)), normalized(normalized) {}

    uint32_t BufferElement::get_component_count() const {
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
            default : return 0;
        }
    }

    BufferLayout::BufferLayout(std::initializer_list<BufferElement> elements) : elements_(elements) {
        calculate_offsets_and_stride();
    }

    void BufferLayout::calculate_offsets_and_stride() {
        size_t offset = 0;
        stride_ = 0;
        for (auto& element : elements_) {
            element.offset = offset;
            offset += element.size;
            stride_ += element.size;
        }
    }
}
