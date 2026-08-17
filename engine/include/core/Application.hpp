//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <core/LayerStack.hpp>
#include <render/Renderer.hpp>

namespace engine {
    class Application {
    public:
        Application();
        virtual ~Application() = default;

        /** @brief Entry point for running the application */
        int run();

        void shutdown(Renderer& renderer, GLFWwindow* window);

        static Application& get() { return *instance; }
        GLFWwindow& get_window() { return *window_; }

    protected:
        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);

    private:
        static Application* instance;
        GLFWwindow* window_;
        Renderer renderer_;
        LayerStack layers_;
    };
}
