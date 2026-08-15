//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <core/LayerStack.hpp>
#include <input/CameraController.hpp>
#include <render/Renderer.hpp>

namespace engine {
    class Application {
    public:
        virtual ~Application() = default;

        /** @brief Entry point for running the application */
        int run();

        virtual void on_init(Renderer& renderer) = 0;

        virtual void on_update(Renderer& renderer, GLFWwindow* window, CameraController& camera_controller, float delta_time) = 0;

        virtual void on_render(Renderer& renderer) = 0;

        void on_shutdown(Renderer& renderer, GLFWwindow* window);

        void push_layer(Layer* layer);
        void push_overlay(Layer* overlay);

    private:
        LayerStack layers_;
    };
}
