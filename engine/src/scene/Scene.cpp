//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "scene/Scene.hpp"

#include "asset/AssetManager.hpp"
#include "core/Framebuffer.hpp"
#include "core/Log.hpp"
#include "core/UUID.hpp"
#include "physics/CollisionSystem.hpp"
#include "render/mesh/InstanceData.hpp"
#include "scene/ecs/Entity.hpp"

namespace engine {
    Scene::Scene() = default;

    Scene::~Scene() = default;

    Entity Scene::create_entity(const std::string& name) {
        return create_entity_with_uuid(UUID(), name);
    }

    Entity Scene::create_entity_with_uuid(UUID uuid, const std::string& name) {
        Entity entity = {registry_.create(), this};
        entity.add_component<IDComponent>(uuid);

        auto& tag = entity.add_component<LabelComponent>();
        tag.tag = name.empty() ? "Entity" : name;

        entity.add_component<ActiveComponent>(true);
        entity.add_component<TransformComponent>();
        CORE_INFO("Created entity - {0} [{1}]", name, static_cast<uint64_t>(uuid));
        return entity;
    }

    void Scene::destroy_entity(Entity entity) {
        CORE_INFO("Deleted entity - {0} [{1}]", entity.name(), static_cast<uint64_t>(entity.uuid()));
        registry_.destroy(entity);
    }

    void Scene::on_init(Renderer& renderer) {
        collision_system_ = std::make_unique<CollisionSystem>(10.0f);

        shadow_framebuffer_ = std::make_unique<Framebuffer>(2048, 2048);

        renderer.get_shader_library().load(
            "shadow_depth",
            "assets/shaders/shadow_depth.vert",
            "assets/shaders/shadow_depth.frag"
        );
    }

    void Scene::on_shutdown() {
        shadow_framebuffer_.reset();
        collision_system_.reset();
    }

    void Scene::on_update(float delta_time) {
        collision_system_->update_broadphase(*this);
    }

    void Scene::on_render(Renderer& renderer, Camera& camera, AssetManager& asset_manager, const float delta_time) {
        renderer.begin_scene(camera);
        Renderer::set_blend(true);

        // ------------------
        // Lights
        // ------------------
        glm::vec3 main_light_dir{0.0f, -1.0f, 0.0f};
        bool has_dir_light = false;

        auto dir_light_view = registry_.view<TransformComponent, DirectionalLightComponent>();
        for (auto entity : dir_light_view) {
            auto [transform, light] = dir_light_view.get<TransformComponent, DirectionalLightComponent>(entity);
            if (registry_.all_of<ActiveComponent>(entity) && !registry_.get<ActiveComponent>(entity).active) {
                continue;
            }

            const glm::vec3 light_forward = transform.get_forward();
            renderer.add_directional_light({
                .direction = light_forward,
                .color     = light.color,
                .intensity = light.intensity
            });

            if (!has_dir_light) {
                main_light_dir = light_forward;
                has_dir_light = true;
            }
        }
        auto point_light_view = registry_.view<TransformComponent, PointLightComponent>();
        for (auto entity : point_light_view) {
            auto [transform, light] = point_light_view.get<TransformComponent, PointLightComponent>(entity);
            if (registry_.all_of<ActiveComponent>(entity) && !registry_.get<ActiveComponent>(entity).active) {
                continue;
            }
            renderer.add_point_light({
                .position  = transform.position,
                .color     = light.color,
                .intensity = light.intensity,
                .radius    = light.radius
            });
        }
        // ------------------
        // Meshes
        // ------------------
        for (auto& batch : mesh_batches_ | std::views::values) {
            batch.clear();
        }

        const Frustum frustum = camera.get_frustum();
        const glm::vec3 camera_pos = camera.get_position();

        auto mesh_view = registry_.view<TransformComponent, MeshComponent, ActiveComponent>();
        for (auto entity : mesh_view) {
            auto [transform, mesh_comp, active] = mesh_view.get<TransformComponent, MeshComponent, ActiveComponent>(entity);

            if (!active.active) continue;

            UUID target_mesh_handle = mesh_comp.handle; // Default to base mesh (LOD0)
            const float distance_to_camera = glm::distance(transform.position, camera_pos);

            if (registry_.all_of<LODComponent>(entity)) {
                const auto& [levels, max_draw_distance] = registry_.get<LODComponent>(entity);
                if (distance_to_camera > max_draw_distance) continue;

                for (const auto& [mesh_handle, max_distance] : levels) {
                    if (distance_to_camera > max_distance) {
                        target_mesh_handle = mesh_handle;
                    }
                    else {
                        break;
                    }
                }
            }

            if (auto mesh = asset_manager.get_asset<Mesh>(target_mesh_handle)) {
                const glm::mat4 model_matrix = transform.get_matrix();
                if (!frustum.intersects(mesh->local_bounds, model_matrix)) continue;

                glm::vec4 color{1.0f};
                float roughness = 0.5f;
                float metallic = 0.0f;
                if (registry_.all_of<MaterialComponent>(entity)) {
                    const auto& mat = registry_.get<MaterialComponent>(entity);
                    color = mat.color;
                    roughness = mat.roughness;
                    metallic = mat.metallic;
                }
                mesh_batches_[target_mesh_handle].push_back(InstanceData{
                    .transform = model_matrix,
                    .color     = color,
                    .pbr       = glm::vec2(roughness, metallic)
                });
            }
        }
        // ------------------
        // Shadow pass
        // ------------------
        glm::mat4 light_space_matrix(1.0f);
        if (has_dir_light) {
            const glm::vec3 light_pos = camera_pos - main_light_dir * 40.0f;

            const glm::vec3 up = std::abs(main_light_dir.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
            const glm::mat4 light_view = glm::lookAt(light_pos, camera_pos, up);
            const glm::mat4 light_projection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, 0.1f, 120.0f);
            light_space_matrix = light_projection * light_view;
        }

        if (has_dir_light && shadow_framebuffer_) {
            GLint prev_fbo = 0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
            GLint prev_viewport[4];
            glGetIntegerv(GL_VIEWPORT, prev_viewport);

            shadow_framebuffer_->bind();
            glClear(GL_DEPTH_BUFFER_BIT);

            auto shadow_shader = renderer.get_shader_library().get("shadow_depth");
            if (shadow_shader) {
                shadow_shader->bind();
                shadow_shader->set_mat4("u_light_space_matrix", light_space_matrix);

                bool prev_culling = renderer.is_frustum_culling_enabled();
                renderer.set_frustum_culling_enabled(false);

                for (const auto& [handle, instance_list] : mesh_batches_) {
                    if (std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle)) {
                        renderer.draw_mesh_instanced(*mesh, instance_list, shadow_shader);
                    }
                }

                renderer.set_frustum_culling_enabled(prev_culling);
            }

            // Restore editor framebuffer and viewport
            glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
            glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
        }

        // PASS 2: Main Lighting Pass
        if (shadow_framebuffer_) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, shadow_framebuffer_->get_depth_attachment_id());

            if (auto main_shader = renderer.get_shader_library().get("mesh")) {
                main_shader->bind();
                main_shader->set_int("u_shadow_map", 3);
                main_shader->set_mat4("u_light_space_matrix", light_space_matrix);
            }
        }

        for (const auto& [handle, instance_list] : mesh_batches_) {
            if (std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle)) {
                renderer.draw_mesh_instanced(*mesh, instance_list);
            }
        }
        renderer.end_scene(delta_time);
    }
}
