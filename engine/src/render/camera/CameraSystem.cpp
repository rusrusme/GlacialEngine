//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/camera/CameraSystem.hpp"

#include "project/scene/Scene.hpp"
#include "project/scene/ecs/Components.hpp"

namespace engine {
    void CameraSystem::on_update(Scene& active_scene, [[maybe_unused]] float delta_time) {
        auto view = active_scene.view<TransformComponent, CameraComponent>(exclude<DisabledComponent>);
        for (auto&& [entity, transform, camera_comp] : view.each()) {
            if (!camera_comp.is_primary) continue;

            controller_.on_update(camera_comp.native);

            transform.position = camera_comp.native.position();
            transform.rotation = {camera_comp.native.pitch(), camera_comp.native.yaw(), 0.0f};
        }
    }

    void CameraSystem::on_event(Scene& active_scene, Event& event) {
        auto view = active_scene.view<TransformComponent, CameraComponent>(exclude<DisabledComponent>);
        for (auto&& [entity, transform, camera_comp] : view.each()) {
            if (!camera_comp.is_primary) continue;

            controller_.on_event(camera_comp.native, event);
            break;
        }
    }
}
