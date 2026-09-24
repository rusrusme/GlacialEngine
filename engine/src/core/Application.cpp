//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "core/Application.hpp"

#include "core/Config.hpp"
#include "core/Log.hpp"
#include "event/MouseEvents.hpp"
#include "event/WindowEvents.hpp"
#include "render/Frame.hpp"

namespace engine {
    Application::Application() {
        instance_ = this;
    }

    int Application::run() {
        Log::init();
        CORE_INFO("Starting application");
        setting::configuration::load();
        if (!glfwInit()) {
            CORE_CRITICAL("Application initialization failed: Could not initialize GLFW.");
            return 1;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        CORE_INFO("Creating window: '{0}' ({1}x{2})",
                  setting::FRAME_TITLE,
                  setting::video::MONITOR_RESOLUTION_WIDTH,
                  setting::video::MONITOR_RESOLUTION_HEIGHT);

        GLFWwindow* raw_window = glfwCreateWindow(
            setting::video::MONITOR_RESOLUTION_WIDTH,
            setting::video::MONITOR_RESOLUTION_HEIGHT,
            setting::FRAME_TITLE, nullptr, nullptr);

        if (!raw_window) {
            CORE_CRITICAL("Application initialization failed: Could not create GLFW window");
            glfwTerminate();
            return 1;
        }
        glfwMakeContextCurrent(raw_window);

        window_ = std::make_unique<Window>(raw_window);
        window_->set_event_callback([this](Event& event) { on_event(event); });

        // ----------------------------------
        // Initialize core systems
        // ----------------------------------
        Frame::init(window_->native());

        if (!renderer_.on_init(window_->native())) {
            CORE_CRITICAL("Application initialization failed: Could not initialize renderer");
            glfwDestroyWindow(window_->native());
            glfwTerminate();
            return 1;
        }
        CORE_INFO("Renderer successfully initialized");

        CORE_INFO("Attaching application layers");
        for (const auto& layer : layers_) {
            layer->on_attach(renderer_);
        }
        CORE_INFO("Application startup complete");

        // ----------------------------------
        // Run core loop
        // ----------------------------------
        while (running_) {
            const float delta_time = Frame::begin_frame();

            // ----------------------------------
            // Update pass
            // ----------------------------------
            for (const auto& layer : layers_) {
                layer->on_update(delta_time);
            }

            // ----------------------------------
            // Render pass
            // ----------------------------------
            for (const auto& layer : layers_) {
                layer->on_render(renderer_, delta_time);
            }
            #ifdef ENABLE_IMGUI
            for (const auto& layer : layers_) {
                layer->on_imgui_render(renderer_, delta_time);
            }
            #endif
            // ----------------------------------

            Frame::end_frame();
        }
        // In reverse
        CORE_INFO("Detaching application layers");
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            (*it)->on_detach();
        }

        shutdown();
        return 0;
    }

    void Application::on_event(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowClosedEvent>([this](const WindowClosedEvent& _) {
            CORE_INFO("Window close request received");
            close();
            return true;
        });

        dispatcher.dispatch<WindowResizedEvent>([this](const WindowResizedEvent& e) {
            CORE_TRACE("Window resized to {0}x{1}", e.width(), e.height());
            renderer_.on_resize(e.width(), e.height());
            return false; // Forward to layer
        });

        // From top to bottom
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            if (event.handled) {
                break; // Event consumed
            }
            (*it)->on_event(event);
        }
    }

    void Application::push_layer(std::unique_ptr<Layer> layer) {
        layers_.push_layer(std::move(layer));
    }

    void Application::push_overlay(std::unique_ptr<Layer> overlay) {
        layers_.push_overlay(std::move(overlay));
    }

    void Application::shutdown() {
        CORE_INFO("Shutting down engine application");

        setting::configuration::save();

        Frame::shutdown();
        renderer_.on_shutdown();
        if (window_ != nullptr) {
            glfwDestroyWindow(window_->native());
        }
        glfwTerminate();
        CORE_INFO("Engine shutdown complete");
    }

    Application* Application::instance_ = nullptr;
}
