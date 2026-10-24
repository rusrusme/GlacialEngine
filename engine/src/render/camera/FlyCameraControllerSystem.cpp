//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/camera/FlyCameraControllerSystem.hpp"

#include <algorithm>

#include "event/Event.hpp"
#include "event/MouseEvents.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "input/MouseCodes.hpp"
#include "project/scene/Scene.hpp"
#include "project/scene/ecs/Components.hpp"

namespace engine {
    void FlyCameraControllerSystem::on_update(Scene& active_scene, float delta_time) {
        const auto mouse_pos = Input::mouse_position();

        auto view = active_scene.view<TransformComponent, CameraComponent, FlyCameraControllerComponent>(exclude<DisabledComponent>);
        for (auto [entity, transform, camera_comp, controller] : view.each()) {
            if (!camera_comp.is_primary) continue;

            if (controller.is_first_frame) {
                controller.last_mouse_position = mouse_pos;
                controller.is_first_frame = false;
                continue;
            }

            const glm::vec2 mouse_delta = mouse_pos - controller.last_mouse_position;
            controller.last_mouse_position = mouse_pos;

            const bool can_rotate = !controller.require_mouse_button ||
                Input::mouse_button_pressed(Mouse::MOUSE_BUTTON_RIGHT);

            if (can_rotate) {
                transform.rotation.y -= mouse_delta.x * controller.mouse_sensitivity;
                transform.rotation.x -= mouse_delta.y * controller.mouse_sensitivity;

                transform.rotation.x = std::clamp(transform.rotation.x, -89.0f, 89.0f);
            }

            glm::vec3 move_direction{0.0f};

            const glm::vec3 forward = transform.get_forward();
            const glm::vec3 right = transform.get_right();
            constexpr glm::vec3 world_up = {0.0f, 1.0f, 0.0f};

            if (Input::key_pressed(Key::W)) move_direction += forward;
            if (Input::key_pressed(Key::S)) move_direction -= forward;
            if (Input::key_pressed(Key::D)) move_direction += right;
            if (Input::key_pressed(Key::A)) move_direction -= right;
            if (Input::key_pressed(Key::E) || Input::key_pressed(Key::SPACE)) move_direction += world_up;
            if (Input::key_pressed(Key::Q) || Input::key_pressed(Key::LEFT_CONTROL)) move_direction -= world_up;

            if (glm::length(move_direction) > 0.0001f) {
                float speed = controller.move_speed;
                if (Input::key_pressed(Key::LEFT_SHIFT)) {
                    speed *= controller.sprint_multiplier;
                }

                transform.position += glm::normalize(move_direction) * (speed * delta_time);
            }
        }
    }

    void FlyCameraControllerSystem::on_event(Scene& active_scene, Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseScrolledEvent>([&active_scene](const MouseScrolledEvent& e) {
            auto view = active_scene.view<TransformComponent, CameraComponent, FlyCameraControllerComponent>(exclude<DisabledComponent>);
            for (auto [entity, transform, camera_comp, controller] : view.each()) {
                if (!camera_comp.is_primary) continue;
                controller.move_speed = std::clamp(controller.move_speed + e.y_offset() * 0.5f, 0.5f, 50.0f);
                break;
            }
            return false;
        });
    }
}
