//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"
#include "render/camera/Camera.hpp"
#include "render/camera/CameraController.hpp"

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

        void set_viewport_hovered(const bool hovered) { viewport_hovered_ = hovered; }
        void set_viewport_focused(const bool focused) { viewport_focused_ = focused; }

    private:
        Camera cam_;
        CameraController cam_controller_;

        bool viewport_hovered_ = false;
        bool viewport_focused_ = false;
    };
}
