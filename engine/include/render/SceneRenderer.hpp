//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    class ShaderLibrary;

    // TODO Temporary setup
    class SceneRenderer {
    public:
        SceneRenderer(ShaderLibrary& shaders,
                      uint32_t viewport_width = 1920,
                      uint32_t viewport_height = 1080);
        ~SceneRenderer() = default;

        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer& operator=(const SceneRenderer&) = delete;
        SceneRenderer(SceneRenderer&&) noexcept = delete;
        SceneRenderer& operator=(SceneRenderer&&) noexcept = delete;

        void begin_scene();
        void submit_instance();
        void end_scene();
    };
}
