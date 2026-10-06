//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/legacy/Shader.hpp"
#include "render/legacy/VertexArray.hpp"

namespace editor {
    class Grid {
    public:
        Grid() = default;
        ~Grid() = default;

        void init(engine::Renderer& renderer);
        void render();

    private:
        std::shared_ptr<engine::Shader> grid_shader_;
        std::unique_ptr<engine::VertexArray> grid_vao_;
        bool initialized_ = false;
    };
}
