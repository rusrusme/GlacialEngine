//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/LayerStack.hpp"

namespace engine {
    // LayerStack calls on_detach() for every layer in its destructor, but Application::run() also explicitly detaches layers before shutdown. This can result in on_detach() being called twice (and potentially double-freeing resources). Consider removing detach logic from the LayerStack destructor (let Application own detach ordering), or ensure detach only happens in one place.
    LayerStack::~LayerStack() = default;

    void LayerStack::push_layer(std::unique_ptr<Layer> layer) {
        layers_.emplace(layers_.begin() + layer_insert_index_, std::move(layer));
        layer_insert_index_++;
    }

    void LayerStack::push_overlay(std::unique_ptr<Layer> overlay) {
        layers_.emplace_back(std::move(overlay));
    }

    std::unique_ptr<Layer> LayerStack::pop_layer(Layer* layer) {
        auto it = std::find_if(layers_.begin(), layers_.begin() + layer_insert_index_, [layer](const std::unique_ptr<Layer>& ptr) { return ptr.get() == layer; });

        if (it != layers_.begin() + layer_insert_index_) {
            (*it)->on_detach();
            std::unique_ptr<Layer> removed = std::move(*it);
            layers_.erase(it);
            layer_insert_index_--;
            return removed;
        }
        return nullptr;
    }

    std::unique_ptr<Layer> LayerStack::pop_overlay(Layer* overlay) {
        auto it = std::find_if(layers_.begin() + layer_insert_index_, layers_.end(), [overlay](const std::unique_ptr<Layer>& ptr) { return ptr.get() == overlay; });

        if (it != layers_.end()) {
            (*it)->on_detach();
            std::unique_ptr<Layer> removed = std::move(*it);
            layers_.erase(it);
            return removed;
        }
        return nullptr;
    }
}
