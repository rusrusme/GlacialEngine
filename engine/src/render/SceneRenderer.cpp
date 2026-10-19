//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/SceneRenderer.hpp"

#include "asset/AssetManager.hpp"
#include "core/SystemContext.hpp"
#include "project/scene/ecs/Entity.hpp"
#include "render/rhi/RenderContext.hpp"
#include "util/Instrumentor.hpp"

namespace engine {
    void SceneRenderer::init(const RHIDevice& device) {
        opaque_pass_.init(device);

        frame_ubo_ = device.create_uniform_buffer(sizeof(glm::mat4), 0);
        frame_descriptor_set_ = device.create_descriptor_set();
        frame_descriptor_set_->write_uniform_buffer(0, *frame_ubo_);

        opaque_queue_.reserve(1024);
    }

    void SceneRenderer::begin_scene(const Camera& camera) {
        begin_scene(camera.view_projection());
    }

    void SceneRenderer::begin_scene(const glm::mat4& view_projection) {
        opaque_queue_.clear();
        frame_ubo_->set_data(&view_projection, sizeof(glm::mat4));
    }

    void SceneRenderer::end_scene(CommandBuffer& cb, Framebuffer& target_framebuffer) {
        opaque_pass_.execute(cb, target_framebuffer, *frame_descriptor_set_, opaque_queue_);
    }

    void SceneRenderer::submit(const RenderObject& object) {
        if (!object.mesh) return;
        opaque_queue_.push_back(object);
    }

    void SceneRenderer::submit(const Mesh* mesh, const glm::mat4& transform, const Texture2D* albedo_texture) {
        if (!mesh) return;
        opaque_queue_.push_back(RenderObject{
            .mesh           = mesh,
            .transform      = transform,
            .albedo_texture = albedo_texture
        });
    }

    void SceneRenderer::render_scene(Scene& scene, const SystemContext& context, const Camera* override_camera) {
        PROFILE_FUNCTION();

        const auto& swapchain = context.render_context.swapchain();
        if (swapchain.width() == 0 || swapchain.height() == 0) return;

        glm::mat4 view_proj{1.0f};

        if (override_camera) {
            view_proj = override_camera->view_projection();
        }
        else {
            bool found_camera = false;
            const float aspect_ratio = static_cast<float>(swapchain.width()) / static_cast<float>(swapchain.height());

            auto camera_view = scene.view<TransformComponent, CameraComponent>(exclude<DisabledComponent>);
            for (const auto& [entity_handle, transform, camera_comp] : camera_view.each()) {
                if (!camera_comp.is_primary) continue;

                const glm::mat4 view = glm::lookAt(
                    transform.position,
                    transform.position + transform.get_forward(),
                    transform.get_up()
                );

                const glm::mat4 proj = camera_comp.projection(aspect_ratio);

                view_proj = proj * view;
                found_camera = true;
                break;
            }

            // Render nothing
            if (!found_camera) {
                return;
            }
        }

        begin_scene(view_proj);

        auto mesh_view = scene.view<TransformComponent, MeshComponent>(exclude<DisabledComponent>);
        for (const auto& [entity_handle, transform, mesh_comp] : mesh_view.each()) {
            const Shared<Mesh> mesh_asset = context.assets.get<Mesh>(mesh_comp.handle);
            if (!mesh_asset) {
                continue;
            }

            const Texture2D* albedo_tex = nullptr;
            Entity entity{entity_handle, &scene};
            if (entity.has_component<MaterialComponent>()) {
                const auto& mat = entity.get_component<MaterialComponent>();
                if (mat.texture_handle != AssetHandle(0)) {
                    if (auto tex_asset = context.assets.get<Texture2D>(mat.texture_handle)) {
                        albedo_tex = tex_asset.get();
                    }
                }
            }

            submit(mesh_asset.get(), transform.get_matrix(), albedo_tex);
        }

        end_scene(
            context.render_context.active_command_buffer(),
            context.render_context.swapchain().backbuffer()
        );
    }
}
