//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "EditorGrid.hpp"

namespace editor {
    void EditorGrid::init(engine::Renderer& renderer) {
        if (initialized_) return;

        grid_shader_ = renderer.get_shader_library().load(
            "grid",
            "assets/shaders/editor/grid.vert",
            "assets/shaders/editor/grid.frag"
        );

        grid_vao_ = std::make_unique<engine::VertexArray>();
        initialized_ = true;
    }

    void EditorGrid::render(const engine::Renderer& renderer) {
        if (!initialized_ || !grid_shader_ || !grid_vao_) return;

        engine::Renderer::set_blend(true);

        grid_shader_->bind();
        engine::Renderer::draw_arrays(*grid_vao_, 6);
        grid_shader_->unbind();

        engine::Renderer::set_blend(false);
    }
}
