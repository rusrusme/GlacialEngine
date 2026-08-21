//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "scene/system/CameraSystem.hpp"

#include "render/Camera.hpp"
#include "scene/ecs/Components.hpp"

namespace engine {
    void CameraSystem::reset_mouse_state() {
        controller_.reset_mouse_state();
    }

    void CameraSystem::update(entt::registry& registry, float delta_time) {
        auto view = registry.view<TransformComponent, CameraComponent>();

        for (auto entity : view) {
            const auto& [transform, camera_comp] = view.get<TransformComponent, CameraComponent>(entity);
            if (!camera_comp.is_primary) continue;

            controller_.update(camera_comp.camera, delta_time);

            transform.position = camera_comp.camera.get_position();
        }
    }

    void CameraSystem::on_scroll(entt::registry& registry, double y_offset) {
        auto view = registry.view<CameraComponent>();
        for (auto entity : view) {
            auto& camera_comp = view.get<CameraComponent>(entity);
            if (camera_comp.is_primary) {
                CameraController::on_scroll(camera_comp.camera, y_offset);
            }
        }
    }
}
