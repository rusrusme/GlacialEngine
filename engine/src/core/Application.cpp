//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include <utility>

#include "core/Application.hpp"

#include "core/Config.hpp"
#include "core/FrameLimiter.hpp"
#include "core/Log.hpp"
#include "core/Timestep.hpp"
#include "core/WindowSystem.hpp"
#include "debug/ImguiLayer.hpp"
#include "event/MouseEvents.hpp"
#include "event/WindowEvents.hpp"
#include "render/rhi/RenderContext.hpp"
#include "util/Instrumentor.hpp"

namespace engine {
    Application::Application(std::string title) : title_(std::move(title)) {
        PROFILE_BEGIN_SESSION("Engine Profile");
        instance_ = this;
    }

    int Application::run() {
        PROFILE_FUNCTION();

        Log::init();
        CORE_INFO("Starting application");
        config::load();

        if (!window::init(GraphicsFramework::GLFW)) {
            CORE_CRITICAL("Application initialization failed: Could not initialize window");
            return 1;
        }

        window_ = std::make_unique<Window>(title_);
        if (!window_->valid()) {
            CORE_CRITICAL("Application initialization failed: Could not create window");
            window::shutdown();
            return 1;
        }
        window_->set_event_callback([this](Event& event) { on_event(event); });

        ///////////////////////////////////////////////////////
        // Initialize core systems
        render_context_.init(*window_);

        // shader_library_ = std::make_unique<ShaderLibrary>();
        // gpu_mesh_manager_ = std::make_unique<GPUMeshManager>(1024 * 1024 * 64, 1024 * 1024 * 16); // 64MB VBO, 16MB IBO
        // material_system_ = std::make_unique<MaterialSystem>(256);
        // texture_system_ = std::make_unique<TextureSystem>(2048, 2048, 256);
        //
        // scene_renderer_ = std::make_unique<SceneRenderer>(
        //     *shader_library_,
        //     *gpu_mesh_manager_,
        //     *material_system_,
        //     *texture_system_,
        //     window_->width(),
        //     window_->height()
        // );
        context_ = std::make_unique<SystemContext>(
            audio_system_,
            asset_manager_,
            scene_manager_,
            shader_library_,
            render_context_,
            *window_
        );

        audio_system_.on_attach();
        ///////////////////////////////////////////////////////

        PROFILE_START("Application::run - Scan Assets");
        asset_manager_.scan_directory("assets");
        PROFILE_END();

        CORE_INFO("Attaching application layers");
        for (const auto& layer : layers_) {
            layer->on_attach(*context_);
        }

        CORE_INFO("Application startup complete");

        Clock clock;
        FrameLimiter frame_limiter;

        ///////////////////////////////////////////////////////
        // Run core loop
        while (running_) {
            const float delta_time = clock.update();

            ///////////////////////////////////////////////////////
            // Update pass
            audio_system_.on_update(*context_, delta_time);

            PROFILE_START("Application::run - Update");
            for (const auto& layer : layers_) {
                layer->on_update(*context_, delta_time);
            }
            PROFILE_END();

            ///////////////////////////////////////////////////////
            // Render pass
            PROFILE_START("Application::run - Render");
            render_context_.begin_frame();
            for (const auto& layer : layers_) {
                layer->on_render(*context_, delta_time);
            }
            render_context_.end_frame();

            // window::swap_buffers(window_->native()); -> Swapchain does that now
            window::poll_events();

            frame_limiter.limit();

            PROFILE_END();
            ///////////////////////////////////////////////////////
        }
        // In reverse
        CORE_INFO("Detaching application layers");
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            (*it)->on_detach(*context_);
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
            render_context_.on_window_resize(e.width(), e.height());
            return false; // Forward to layer
        });

        // From top to bottom
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            if (event.handled) {
                break; // Event consumed
            }
            (*it)->on_event(*context_, event);
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

        scene_manager_.on_detach();
        if (context_) {
            audio_system_.on_detach(*context_);
        }

        context_.reset();
        render_context_.shutdown();
        window_.reset();

        config::save();
        window::shutdown();
        CORE_INFO("Engine shutdown complete");
    }

    Application* Application::instance_ = nullptr;
}
