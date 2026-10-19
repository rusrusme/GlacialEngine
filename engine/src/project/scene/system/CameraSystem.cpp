//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/system/CameraSystem.hpp"

#include "project/scene/Scene.hpp"

namespace engine {
    void CameraSystem::on_update(const std::shared_ptr<Scene>& scene, float delta_time) {
        if (!scene) return;

        auto view = scene->view<TransformComponent, CameraComponent>(entt::exclude<DisabledComponent>);
        for (const auto& [entity, transform, camera_comp] : view.each()) {
            if (!camera_comp.is_primary) continue;

            // controller_.on_update(camera_comp.camera);
            // transform.position = camera_comp.camera.position();
        }
    }

    void CameraSystem::on_event(const std::shared_ptr<Scene>& scene, double y_offset) {
        auto view = scene->view<CameraComponent>();
        for (auto entity : view) {
            auto& camera_comp = view.get<CameraComponent>(entity);
            if (camera_comp.is_primary) {
                // CameraController::on_scroll(camera_comp.camera, y_offset);
            }
        }
    }
}
