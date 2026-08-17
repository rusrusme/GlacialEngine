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

        void shutdown(Renderer& renderer) const;

        static Application& get() { return *instance_; }
        [[nodiscard]] GLFWwindow* get_window() const { return window_; }

    protected:
        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);

    private:
        static Application* instance_;
        GLFWwindow* window_ = nullptr;
        Renderer renderer_;
        LayerStack layers_;
    };
}
