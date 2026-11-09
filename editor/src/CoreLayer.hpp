//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "core/Layer.hpp"
#include "render/camera/Camera.hpp"
#include "render/camera/CameraController.hpp"
#include "render/rhi/Framebuffer.hpp"
#include "util/ViewportGrid.hpp"

namespace editor {
    using namespace engine;

    class CoreLayer : public Layer {
    public:
        void on_attach(SystemContext& context) override;
        void on_detach(SystemContext& context) override;

        void on_update(SystemContext& context, float delta_time) override;
        void on_render(SystemContext& context, float delta_time) override;
        void on_event(SystemContext& context, Event& event) override;

        [[nodiscard]] Camera& camera() { return cam_; }
        [[nodiscard]] Framebuffer* framebuffer() const { return framebuffer_.get(); }
        [[nodiscard]] ViewportGrid& grid() { return grid_; }

    private:
        Camera cam_;
        CameraController cam_controller_;
        bool controlling_cam_ = false;

        std::unique_ptr<Framebuffer> framebuffer_{nullptr};

        ViewportGrid grid_;
    };
}
