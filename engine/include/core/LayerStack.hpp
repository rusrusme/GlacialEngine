//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "Layer.hpp"

namespace engine {
    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack();

        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);

        std::unique_ptr<Layer> pop_layer(Layer* layer);
        std::unique_ptr<Layer> pop_overlay(Layer* overlay);

        std::vector<std::unique_ptr<Layer>>::iterator begin() { return layers_.begin(); }
        std::vector<std::unique_ptr<Layer>>::iterator end() { return layers_.end(); }
        [[nodiscard]] std::vector<std::unique_ptr<Layer>>::const_iterator begin() const { return layers_.begin(); }
        [[nodiscard]] std::vector<std::unique_ptr<Layer>>::const_iterator end() const { return layers_.end(); }

        std::vector<std::unique_ptr<Layer>>::reverse_iterator rbegin() { return layers_.rbegin(); }
        std::vector<std::unique_ptr<Layer>>::reverse_iterator rend() { return layers_.rend(); }
        [[nodiscard]] std::vector<std::unique_ptr<Layer>>::const_reverse_iterator rbegin() const { return layers_.rbegin(); }
        [[nodiscard]] std::vector<std::unique_ptr<Layer>>::const_reverse_iterator rend() const { return layers_.rend(); }

    private:
        std::vector<std::unique_ptr<Layer>> layers_;
        uint32_t layer_insert_index_ = 0;
    };
}
