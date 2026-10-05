//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include <utility>

#include "core/Application.hpp"

#include "core/Config.hpp"
#include "core/Log.hpp"
#include "core/WindowAPI.hpp"
#include "event/MouseEvents.hpp"
#include "event/WindowEvents.hpp"
#include "render/Frame.hpp"
#include "render/debug/DebugRenderer.hpp"
#include "util/Instrumentor.hpp"

namespace engine {
    Application::Application() : Application(setting::ENGINE_TITLE_) {}

    Application::Application(std::string title) : title_(std::move(title)) {
        PROFILE_BEGIN_SESSION("Engine_Profile", "profile_results.json");
        instance_ = this;
    }

    int Application::run() {
        PROFILE_FUNCTION();

        Log::init();
        CORE_INFO("Starting application");
        setting::configuration::load();

        if (!window::init(GraphicsFramework::GLFW)) {
            CORE_CRITICAL("Application initialization failed: Could not initialize window");
            return 1;
        }

        window_ = std::make_unique<Window>(title_);
        if (!window_->is_valid()) {
            CORE_CRITICAL("Application initialization failed: Could not create window");
            window::shutdown();
            return 1;
        }
        window_->set_event_callback([this](Event& event) { on_event(event); });

        // ----------------------------------
        // Initialize core systems
        // ----------------------------------
        Frame::init(window_->native());
        if (!renderer_.on_attach(window_->native())) {
            CORE_CRITICAL("Application initialization failed: Could not initialize renderer");
            window_.reset();
            window::shutdown();
            return 1;
        }
        DebugRenderer::on_attach();

        audio_system_.on_attach();

        PROFILE_START(asset_timer, "Scanning initial asset directory");
        asset_manager_.scan_directory("assets");
        PROFILE_END(asset_timer);

        CORE_INFO("Attaching application layers");
        for (const auto& layer : layers_) {
            layer->on_attach(context_);
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
            audio_system_.on_update(context_, delta_time);

            PROFILE_START(update_timer, "Layers - On Update");
            for (const auto& layer : layers_) {
                layer->on_update(context_, delta_time);
            }
            PROFILE_END(update_timer);

            // ----------------------------------
            // Render pass
            // ----------------------------------
            PROFILE_START(render_timer, "Layers - On Render");
            for (const auto& layer : layers_) {
                layer->on_render(context_, delta_time);
            }
            PROFILE_END(render_timer);

            #ifdef ENABLE_IMGUI
            PROFILE_START(imgui_timer, "Layers - On Imgui Render");
            for (const auto& layer : layers_) {
                layer->on_imgui_render(context_, delta_time);
            }
            PROFILE_END(imgui_timer);
            #endif
            // ----------------------------------

            Frame::end_frame();
        }
        // In reverse
        CORE_INFO("Detaching application layers");
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            (*it)->on_detach(context_);
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
            (*it)->on_event(context_, event);
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

        Frame::shutdown();
        DebugRenderer::on_detach();
        renderer_.on_detach();
        scene_manager_.on_detach();
        audio_system_.on_detach(context_);

        window_.reset();

        setting::configuration::save();
        window::shutdown();
        CORE_INFO("Engine shutdown complete");
    }

    Application* Application::instance_ = nullptr;
}
