//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/RenderScope.hpp"

#include "render/legacy/debug/DebugRenderer.hpp"

namespace engine {
    RenderScope::RenderScope(Renderer& renderer, const float delta_time) : renderer_(renderer), delta_time_(delta_time) {}

    RenderScope::~RenderScope() {
        if (is_active_) end();
    }

    void RenderScope::begin() {
        if (is_active_) return;
        renderer_.begin_scene();
        is_active_ = true;
    }

    void RenderScope::begin(const Camera& camera) {
        if (!is_active_) begin();
        renderer_.bind_camera(camera);
        DebugRenderer::begin_scene(camera);
        has_camera_ = true;
    }

    void RenderScope::end() {
        if (!is_active_) return;

        DebugRenderer::end_scene();
        renderer_.end_scene();

        if (renderer_.should_render_to_screen()) {
            renderer_.render();
        }
        is_active_ = false;
        has_camera_ = false;
    }
}
