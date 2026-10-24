//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/SceneRenderer.hpp"

#include "asset/AssetManager.hpp"
#include "core/SystemContext.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "render/ShaderData.hpp"
#include "render/rhi/RenderContext.hpp"
#include "util/Instrumentor.hpp"

namespace engine {
    void SceneRenderer::init(const RHIDevice& device) {
        opaque_pass_.init(device);

        frame_ubo_ = device.create_uniform_buffer(sizeof(FrameUniformData), 0);
        frame_descriptor_set_ = device.create_descriptor_set();
        frame_descriptor_set_->write_uniform_buffer(0, *frame_ubo_);

        opaque_queue_.reserve(1024);
    }

    void SceneRenderer::shutdown() {
        opaque_pass_.shutdown();

        frame_descriptor_set_.reset();
        frame_ubo_.reset();
        opaque_queue_.clear();
    }

    void SceneRenderer::begin_scene(const FrameUniformData& frame_data) {
        opaque_queue_.clear();
        frame_ubo_->set_data(&frame_data, sizeof(FrameUniformData));
    }

    void SceneRenderer::end_scene(CommandBuffer& cb, Framebuffer& target_framebuffer) {
        opaque_pass_.execute(cb, target_framebuffer, *frame_descriptor_set_, opaque_queue_);
    }

    void SceneRenderer::submit(const RenderObject& object) {
        if (!object.mesh) return;
        opaque_queue_.push_back(object);
    }

    void SceneRenderer::submit(const Mesh* mesh,
                               const glm::mat4& transform,
                               const Texture2D* albedo_texture,
                               const Texture2D* normal_texture,
                               const Texture2D* metallic_roughness_texture,
                               const glm::vec4& albedo_color,
                               float metallic,
                               float roughness) {
        if (!mesh) return;
        opaque_queue_.push_back(RenderObject{
            .mesh                       = mesh,
            .transform                  = transform,
            .color                      = albedo_color,
            .albedo_texture             = albedo_texture,
            .normal_texture             = normal_texture,
            .metallic_roughness_texture = metallic_roughness_texture,
            .metallic                   = metallic,
            .roughness                  = roughness
        });
    }

    void SceneRenderer::render_scene(const Shared<Scene>& scene, const SystemContext& context, float delta_time, const Camera* override_camera) {
        PROFILE_FUNCTION();
        if (!scene) return;

        const auto& swapchain = context.render_context.swapchain();
        if (swapchain.width() == 0 || swapchain.height() == 0) return;

        FrameUniformData frame_data{};
        frame_data.viewport_size = {static_cast<float>(swapchain.width()), static_cast<float>(swapchain.height())};

        total_time_ += delta_time;
        frame_data.time = total_time_;
        frame_data.delta_time = delta_time;

        if (override_camera) {
            frame_data.view_projection = override_camera->view_projection();
            frame_data.view = override_camera->view();
            frame_data.projection = override_camera->projection();
            frame_data.camera_position = glm::vec4(override_camera->position(), 1.0f);
        }
        else {
            bool found_camera = false;
            const float aspect_ratio = static_cast<float>(swapchain.width()) / static_cast<float>(swapchain.height());

            auto camera_view = scene->view<TransformComponent, CameraComponent>(exclude<DisabledComponent>);
            for (const auto& [entity_handle, transform, camera_comp] : camera_view.each()) {
                if (!camera_comp.is_primary) continue;

                camera_comp.set_aspect_ratio(aspect_ratio);

                frame_data.view_projection = camera_comp.view_projection();
                frame_data.view = camera_comp.view();
                frame_data.projection = camera_comp.projection();
                frame_data.camera_position = {transform.position, 1.0f}; // Fourth coord. is padding

                found_camera = true;
                break;
            }

            // Render nothing
            if (!found_camera) {
                return;
            }
        }

        begin_scene(frame_data);

        const auto mesh_view = scene->view<TransformComponent, MeshComponent>(exclude<DisabledComponent>);
        for (const auto& [entity_handle, transform, mesh_comp] : mesh_view.each()) {
            const Shared<Mesh> mesh_asset = context.assets.get<Mesh>(mesh_comp.handle);
            if (!mesh_asset) {
                continue;
            }

            const Texture2D* albedo_texture = nullptr;
            const Texture2D* normal_texture = nullptr;
            const Texture2D* metallic_roughness_texture = nullptr;
            glm::vec4 albedo_color{1.0f};
            float metallic = 0.0f;
            float roughness = 1.0f;

            Entity entity{entity_handle, scene.get()};
            if (entity.has_component<MaterialComponent>()) {
                const auto& material = entity.get_component<MaterialComponent>();
                albedo_color = material.color;
                metallic = material.metallic;
                roughness = material.roughness;

                if (material.texture_handle != AssetHandle(0)) {
                    if (auto texture_asset = context.assets.get<Texture2D>(material.texture_handle)) {
                        albedo_texture = texture_asset.get();
                    }
                }

                if (material.normal_texture_handle != AssetHandle(0)) {
                    if (auto normal_asset = context.assets.get<Texture2D>(material.normal_texture_handle)) {
                        normal_texture = normal_asset.get();
                    }
                }

                if (material.metallic_roughness_texture_handle != AssetHandle(0)) {
                    if (auto mr_asset = context.assets.get<Texture2D>(material.metallic_roughness_texture_handle)) {
                        metallic_roughness_texture = mr_asset.get();
                    }
                }
            }

            submit(
                mesh_asset.get(),
                transform.get_matrix(),
                albedo_texture,
                normal_texture,
                metallic_roughness_texture,
                albedo_color,
                metallic,
                roughness
            );
        }

        end_scene(
            context.render_context.active_command_buffer(),
            context.render_context.swapchain().backbuffer()
        );
    }
}
