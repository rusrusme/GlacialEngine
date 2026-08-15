//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "../../include/core/LayerStack.hpp"

namespace engine {
    // LayerStack calls on_detach() for every layer in its destructor, but Application::run() also explicitly detaches layers before shutdown. This can result in on_detach() being called twice (and potentially double-freeing resources). Consider removing detach logic from the LayerStack destructor (let Application own detach ordering), or ensure detach only happens in one place.
    LayerStack::~LayerStack() {
        for (auto& layer : layers_) {
            layer->on_detach();
        }
        layers_.clear();
    }

    void LayerStack::push_layer(Layer* layer) {
        layers_.emplace(layers_.begin() + layer_insert_index, layer);
        layer_insert_index++;
    }

    void LayerStack::push_overlay(Layer* overlay) {
        layers_.emplace_back(overlay);
    }

    void LayerStack::pop_layer(Layer* layer) {
        const auto it = std::find_if(layers_.begin(), layers_.begin() + layer_insert_index,
            [layer](const std::unique_ptr<Layer>& ptr) { return ptr.get() == layer; });

        if (it != layers_.begin() + layer_insert_index) {
            layer->on_detach();
            layers_.erase(it);
            layer_insert_index--;
        }
    }

    void LayerStack::pop_overlay(Layer* overlay) {
        const auto it = std::find_if(layers_.begin() + layer_insert_index, layers_.end(),
            [overlay](const std::unique_ptr<Layer>& ptr) { return ptr.get() == overlay; });

        if (it != layers_.end()) {
            overlay->on_detach();
            layers_.erase(it);
        }
    }
}
