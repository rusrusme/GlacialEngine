//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/SystemContext.hpp"
#include "project/scene/ecs/Components.hpp"
#include "ui/panel/Hierarchy.hpp"

namespace editor {
    using namespace engine;

    class GizmoOverlay {
    public:
        void on_render_overlay(SystemContext& context);

        void set_snap(bool enable) noexcept { snap_enabled_ = enable; }
        [[nodiscard]] bool snap_enabled() const noexcept { return snap_enabled_; }

        void set_enabled(bool enabled) noexcept { enabled_ = enabled; }
        [[nodiscard]] bool enabled() const noexcept { return enabled_; }

        void set_operation(ImGuizmo::OPERATION op) noexcept {
            operation_ = op;
            enabled_ = true;
        }

        [[nodiscard]] ImGuizmo::OPERATION operation() const noexcept { return operation_; }

        void set_mode(ImGuizmo::MODE mode) noexcept { mode_ = mode; }
        [[nodiscard]] ImGuizmo::MODE mode() const noexcept { return mode_; }
        void toggle_mode() noexcept { mode_ = (mode_ == ImGuizmo::LOCAL) ? ImGuizmo::WORLD : ImGuizmo::LOCAL; }

        void cancel_drag(SystemContext& context);

        [[nodiscard]] bool is_using() const noexcept { return ImGuizmo::IsUsing(); }
        [[nodiscard]] bool is_over() const noexcept { return ImGuizmo::IsOver(); }

    private:
        void draw_gizmo(SystemContext& context, Entity entity);

    private:
        bool enabled_{true};
        ImGuizmo::OPERATION operation_{ImGuizmo::TRANSLATE};
        ImGuizmo::MODE mode_{ImGuizmo::LOCAL};

        bool snap_enabled_{false};
        float snap_translation_values_[3]{0.5f, 0.5f, 0.5f};
        float snap_rotation_value_{45.0f};
        float snap_scale_value_{0.5f};

        TransformComponent initial_transform_{};
        bool was_using_gizmo_{false};
        bool gizmo_canceled_{false};
    };
}
