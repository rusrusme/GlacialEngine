//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <input/CameraController.hpp>
#include <render/Renderer.hpp>

namespace engine {
    class Application {
    public:
        virtual ~Application() = default;

        int run();

        virtual void init(Renderer& renderer) = 0;

        virtual void update(Renderer& renderer, GLFWwindow* window, CameraController& camera_controller, float delta_time) = 0;

        virtual void render(Renderer& renderer) = 0;

        static void shutdown(Renderer& renderer, GLFWwindow* window);
    };
}
