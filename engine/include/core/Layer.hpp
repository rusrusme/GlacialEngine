//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include <input/CameraController.hpp>
#include <render/Renderer.hpp>

namespace engine {
    class Layer {
    public:
        explicit Layer(std::string name = "Layer") : debug_name_(std::move(name)) {}
        virtual ~Layer() = default;

        virtual void on_attach(Renderer& renderer) = 0;
        virtual void on_detach() = 0;
        virtual void on_update(float delta_time) = 0;
        virtual void on_render(Renderer& renderer, float delta_time) = 0;
        virtual void on_imGuiRender(float delta_time) = 0;

    protected:
        std::string debug_name_;
    };
};
