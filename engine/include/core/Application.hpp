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
#include "render/SceneRenderer.hpp"
#include "render/gpu/GPUMeshManager.hpp"
#include "render/gpu/MaterialSystem.hpp"
#include "render/gpu/TextureSystem.hpp"
#include "render/shader/ShaderLibrary.hpp"

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
        [[nodiscard]] SystemContext& context() { return *context_; }

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

        // Rendering Core Subsystems
        std::unique_ptr<ShaderLibrary> shader_library_;
        std::unique_ptr<GPUMeshManager> gpu_mesh_manager_;
        std::unique_ptr<MaterialSystem> material_system_;
        std::unique_ptr<TextureSystem> texture_system_;
        std::unique_ptr<SceneRenderer> scene_renderer_;

        std::optional<SystemContext> context_;

        LayerStack layers_;

        bool running_ = true;
    };
}
