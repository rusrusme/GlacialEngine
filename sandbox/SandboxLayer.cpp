//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "SandboxLayer.hpp"

#include "core/Application.hpp"
#include "input/Input.hpp"
#include "input/KeyCodes.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "render/rhi/RenderContext.hpp"
#include "script/ScriptEngine.hpp"

// TODO Add ScriptComponent in Component.hpp
void SandboxLayer::on_attach(SystemContext& context) {
    #if defined(_WIN32)
    ScriptEngine::load_library("assets/scripts/Scripts.dll");
    #else
    ScriptEngine::load_library("assets/scripts/Scripts.so");
    #endif

    auto scene = context.scenes.create("Sandbox Scene");

    {
        auto ground = scene->create_entity("Ground");
        auto ground_mesh = context.assets.asset_handle_from_path("primitive://plane");
        ground.add_component<MeshComponent>(ground_mesh);

        auto& ground_transform = ground.get_component<TransformComponent>();
        ground_transform.scale = {100.0f, 1.0f, 100.0f};

        auto& ground_mat = ground.add_component<MaterialComponent>();
        ground_mat.color = glm::vec4(0.2f, 0.2f, 0.22f, 1.0f);

        const auto car_asset = context.assets.asset_handle_from_path("assets/models/red_car.glb");

        auto car = scene->create_entity("Vehicle");

        auto& transform = car.get_component<TransformComponent>();
        transform.position = {1.0f, 0.0f, 1.0f};
        car.add_component<MeshComponent>(car_asset);
        auto& material = car.add_component<MaterialComponent>();
        material.color = glm::vec4(1.0f);
        material.metallic = 0.0f;
        material.roughness = 1.0f;
        car.add_component<RigidBodyComponent>();
    }

    camera_.set_far_clip(1000);
    camera_.set_near_clip(0.1f);
    camera_.set_fov(60);
    camera_.set_distance(4);

    const auto& swapchain = context.render_context.swapchain();
    camera_.set_viewport_size(swapchain.width(), swapchain.height());
}

void SandboxLayer::on_detach(SystemContext& context) {
}

void SandboxLayer::on_update(SystemContext& context, float delta_time) {
    auto scene = context.scenes.active_scene();
    if (!scene) return;

    if (Input::key_pressed_once(Key::F5)) {
        ScriptEngine::reload();
    }
    ScriptEngine::on_update(scene.get());

    if (Input::key_pressed_once(Key::SPACE)) {
        if (context.scenes.scene_state() == SceneState::Edit) {
            context.scenes.on_play();
        }
        else {
            context.scenes.on_stop();
        }
    }

    if (context.scenes.scene_state() == SceneState::Play) {
        Entity car{};
        auto view = scene->view<LabelComponent>(exclude<DisabledComponent>);
        for (const auto& [entity, label] : view.each()) {
            if (label.tag == "Vehicle") {
                car = {entity, scene.get()};
                break;
            }
        }

        if (car) {
            auto& transform = car.get_component<TransformComponent>();

            constexpr float acceleration = 45.0f; // Acceleration
            constexpr float max_speed = 20.0f;    // Max forward speed
            constexpr float max_reverse = 14.0f;  // Max reverse speed
            constexpr float turn_speed = 110.0f;  // Turn speed (deg/sec)
            constexpr float friction = 8.0f;      // Rolling friction

            // 1. Throttle & Reverse (W / S)
            if (Input::key_pressed(Key::W)) {
                current_speed_ -= acceleration * delta_time;
            }
            else if (Input::key_pressed(Key::S)) {
                current_speed_ += acceleration * delta_time;
            }
            else {
                if (current_speed_ > 0.0f) {
                    current_speed_ = std::max(0.0f, current_speed_ - friction * delta_time);
                }
                else if (current_speed_ < 0.0f) {
                    current_speed_ = std::min(0.0f, current_speed_ + friction * delta_time);
                }
            }

            current_speed_ = std::clamp(current_speed_, -max_reverse, max_speed);

            // 2. Steering (A / D)
            if (std::abs(current_speed_) > 0.1f) {
                const float direction = current_speed_ > 0.0f ? 1.0f : -1.0f;

                if (Input::key_pressed(Key::A)) {
                    transform.rotation.y -= turn_speed * delta_time * direction;
                }
                if (Input::key_pressed(Key::D)) {
                    transform.rotation.y += turn_speed * delta_time * direction;
                }
            }

            // 3. Apply forward movement
            const glm::vec3 forward = transform.get_forward();
            transform.position += forward * (current_speed_ * delta_time);

            // 4. Smooth Position Tracking (Camera Lag)
            constexpr float pos_lag_speed = 5.0f;
            const float pos_factor = 1.0f - std::exp(-pos_lag_speed * delta_time);
            camera_target_ = glm::mix(camera_target_, transform.position, pos_factor);
            camera_.set_target(camera_target_);

            // 5. Smooth Rotation Tracking (Follow Car Heading)
            // If the camera sits in front of the car instead of behind, change this to: (transform.rotation.y + 180.0f)
            const float target_yaw = transform.rotation.y + 180.0f;

            // std::remainder calculates the shortest angle difference within [-180, 180] degrees
            const float angle_diff = std::remainder(target_yaw - camera_.yaw(), 360.0f);

            constexpr float rot_lag_speed = 4.0f; // Higher = stiffer, Lower = looser camera rotation
            const float rot_factor = 1.0f - std::exp(-rot_lag_speed * delta_time);
            camera_.orbit(angle_diff * rot_factor, 0.0f);
        }
    }
    else {
        camera_controller_.on_update(camera_);
    }

    context.scenes.on_update(delta_time);
}

void SandboxLayer::on_render(SystemContext& context, float delta_time) {
    auto active_scene = context.scenes.active_scene();
    if (!active_scene) return;

    const auto& swapchain = context.render_context.swapchain();
    if (swapchain.width() == 0 || swapchain.height() == 0) return;

    camera_.set_viewport_size(swapchain.width(), swapchain.height());

    context.renderer.render_scene(active_scene, context, delta_time, &camera_);
}

void SandboxLayer::on_event(SystemContext& context, Event& event) {
    camera_controller_.on_event(camera_, event);
    if (!event.handled) {
        context.scenes.on_event(event);
    }
}
