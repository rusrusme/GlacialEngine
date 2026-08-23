//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include "core/LayerStack.hpp"
#include "core/Window.hpp"
#include "render/Renderer.hpp"

namespace engine {
    class Application {
    public:
        Application();
        virtual ~Application() = default;

        int run();
        void close() { running_ = false; }

        void on_event(Event& event);

        static Application& get() { return *instance_; }
        [[nodiscard]] Window& window() const { return *window_; }

    protected:
        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);

    private:
        void shutdown();

        static Application* instance_;
        std::unique_ptr<Window> window_;
        Renderer renderer_;
        LayerStack layers_;

        bool running_ = true;
    };
}
