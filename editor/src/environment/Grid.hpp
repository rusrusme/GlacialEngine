//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/Renderer.hpp"
#include "render/Shader.hpp"
#include "render/VertexArray.hpp"

namespace editor {
    class Grid {
    public:
        Grid() = default;
        ~Grid() = default;

        void init(engine::Renderer& renderer);
        void render(const engine::Renderer& renderer);

    private:
        std::shared_ptr<engine::Shader> grid_shader_;
        std::unique_ptr<engine::VertexArray> grid_vao_;
        bool initialized_ = false;
    };
}
