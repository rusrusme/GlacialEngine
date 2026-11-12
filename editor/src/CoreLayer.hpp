//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "EditorSettings.hpp"
#include "core/Layer.hpp"
#include "render/camera/CameraController.hpp"
#include "render/rhi/Framebuffer.hpp"
#include "scene/ViewportGrid.hpp"

namespace editor {
    using namespace engine;

    class CameraSettings : public SettingsCategory {
    public:
        ~CameraSettings() override = default;

        float orbit_speed{0.5f};
        float pan_speed{0.5f};
        float zoom_speed{0.5f};

        [[nodiscard]] std::string_view name() const override { return "Camera"; }

        void on_render_ui(SystemContext& context) override;

        void load() override;
        void save() override;
    };

    class CoreLayer : public Layer {
    public:
        void on_attach(SystemContext& context) override;
        void on_detach(SystemContext& context) override;

        void on_update(SystemContext& context, float delta_time) override;
        void on_render(SystemContext& context, float delta_time) override;
        void on_event(SystemContext& context, Event& event) override;

        [[nodiscard]] Framebuffer* framebuffer() const { return framebuffer_.get(); }
        [[nodiscard]] ViewportGrid& grid() { return grid_; }

    private:
        bool controlling_cam_ = false;

        std::unique_ptr<Framebuffer> framebuffer_{nullptr};

        ViewportGrid grid_;
    };
}
