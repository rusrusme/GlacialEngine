//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include "asset/AssetManager.hpp"
#include "audio/AudioSystem.hpp"
#include "core/LayerStack.hpp"
#include "core/SystemContext.hpp"
#include "core/Window.hpp"
#include "project/scene/SceneManager.hpp"

namespace engine {
    class Application {
    public:
        Application();
        explicit Application(std::string title);
        virtual ~Application() = default;

        int run();
        void close() { running_ = false; }

        void on_event(Event& event);

        static Application& get() { return *instance_; }
        [[nodiscard]] Window& window() const { return *window_; }
        [[nodiscard]] SystemContext& context() { return context_; }

    protected:
        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);


    private:
        void shutdown();

        static Application* instance_;
        std::string title_;

        std::unique_ptr<Window> window_;

        AudioSystem audio_system_;
        AssetManager asset_manager_;
        SceneManager scene_manager_;
        // Renderer renderer_;
        // Todo render: renderer missing -> replace with new architecture
        SystemContext context_{/*.renderer = renderer_,*/ .audio = audio_system_, .assets = asset_manager_, .scenes = scene_manager_};

        LayerStack layers_;

        bool running_ = true;
    };
}
