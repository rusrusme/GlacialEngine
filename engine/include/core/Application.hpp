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
#include "render/shader/ShaderLibrary.hpp"

namespace engine {
    class Application {
    public:
        explicit Application(std::string title = config::ENGINE_TITLE);
        virtual ~Application() = default;

        int run();
        void close() { running_ = false; }

        void on_event(Event& event);

        static Application& get() { return *instance_; }
        [[nodiscard]] Window& window() const { return *window_; }
        [[nodiscard]] SystemContext& context() const { return *context_; }

    protected:
        void push_layer(std::unique_ptr<Layer> layer);
        void push_overlay(std::unique_ptr<Layer> overlay);

    private:
        void shutdown();

        std::string title_;
        std::unique_ptr<Window> window_;

        LayerStack layers_;

        // Core Subsystems
        AudioSystem audio_system_;
        AssetManager asset_manager_;
        SceneManager scene_manager_;
        ShaderLibrary shader_library_;

        // TODO broken mess
        // GPUMeshManager gpu_mesh_manager_{};
        // MaterialSystem material_system_{};
        // TextureSystem texture_system_{};
        // SceneRenderer scene_renderer_;

        std::unique_ptr<SystemContext> context_;

        bool running_ = true;
        static Application* instance_;
    };
}
