//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/rhi/BufferLayout.hpp"

namespace engine {
    BufferLayout::BufferLayout(std::initializer_list<BufferElement> elements)
        : elements_(elements) {
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
