//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "Renderer.hpp"

namespace engine::legacy {
    using namespace engine;
    class RenderScope {
    public:
        RenderScope(Renderer& renderer, float delta_time);
        ~RenderScope();

        void begin();
        void begin(const Camera& camera);

        void end();

    private:
        Renderer& renderer_;
        float delta_time_;
        bool is_active_ = false;
        bool has_camera_ = false;
    };
}
