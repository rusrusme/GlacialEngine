//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/camera/CameraSystem.hpp"

#include "event/Event.hpp"
#include "event/WindowEvents.hpp"
#include "project/scene/Scene.hpp"
#include "project/scene/ecs/Components.hpp"

namespace engine {
    void CameraSystem::on_update(Scene& active_scene, [[maybe_unused]] float delta_time) {
        auto view = active_scene.view<TransformComponent, CameraComponent>(exclude<DisabledComponent>);
        for (auto&& [entity, transform, camera_comp] : view.each()) {
            camera_comp.update_matrices(transform);
        }
    }

    void CameraSystem::on_event(Scene& active_scene, Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizedEvent>([&active_scene](const WindowResizedEvent& e) {
            if (e.width() == 0 || e.height() == 0) return false;

            auto view = active_scene.view<TransformComponent, CameraComponent>(exclude<DisabledComponent>);
            for (auto&& [entity, transform, camera_comp] : view.each()) {
                camera_comp.set_viewport_size(e.width(), e.height());
                camera_comp.update_matrices(transform);
            }
            return false;
        });
    }
}
