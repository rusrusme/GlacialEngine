//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/system/CameraSystem.hpp"

#include <complex>

#include "project/scene/Scene.hpp"
#include "project/scene/ecs/Components.hpp"
#include "render/Camera.hpp"

namespace engine {
    void CameraSystem::reset_mouse_state() {
        controller_.reset_mouse_state();
    }

    void CameraSystem::update(const std::shared_ptr<Scene>& scene, float delta_time) {
        if (!scene) return;
        auto view = scene->view<TransformComponent, CameraComponent>();

        for (auto entity : view) {
            const auto& [transform, camera_comp] = view.get<TransformComponent, CameraComponent>(entity);
            if (!camera_comp.is_primary) continue;

            controller_.update(camera_comp.camera, delta_time);

            transform.position = camera_comp.camera.get_position();
        }
    }

    void CameraSystem::on_scroll(const std::shared_ptr<Scene>& scene, double y_offset) {
        auto view = scene->view<CameraComponent>();
        for (auto entity : view) {
            auto& camera_comp = view.get<CameraComponent>(entity);
            if (camera_comp.is_primary) {
                CameraController::on_scroll(camera_comp.camera, y_offset);
            }
        }
    }
}
