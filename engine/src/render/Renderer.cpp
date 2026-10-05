//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "render/Renderer.hpp"

#include "asset/AssetManager.hpp"
#include "core/Config.hpp"
#include "core/WindowAPI.hpp"
#include "render/Buffer.hpp"
#include "render/Camera.hpp"
#include "render/Frustum.hpp"
#include "render/RenderCommand.hpp"
#include "render/Texture2D.hpp"
#include "render/VertexArray.hpp"
#include "render/mesh/InstanceData.hpp"

namespace engine {
    Renderer::Renderer() = default;
    Renderer::~Renderer() = default;

    bool Renderer::on_attach(void* window) {
        window_ = window;

        if (window_ == nullptr) {
            CORE_CRITICAL("Renderer failed: window pointer is null");
            return false;
        }

        RenderCommand::init();

        window::get_framebuffer_size(window_, &framebuffer_width_, &framebuffer_height_);

        RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(framebuffer_width_), static_cast<uint32_t>(framebuffer_height_));

        post_processor_ = std::make_unique<PostProcessor>();
        post_processor_->init(framebuffer_width_, framebuffer_height_);

        camera_ubo_ = std::make_unique<UniformBuffer>(sizeof(CameraData), 0);

        shader_library_.load("mesh", "assets/shaders/engine/mesh.vert", "assets/shaders/engine/mesh.frag");
        shader_library_.load("depth_prepass", "assets/shaders/engine/depth_prepass.vert", "assets/shaders/engine/depth_prepass.frag");
        shader_library_.load("shadow_depth", "assets/shaders/engine/shadow/shadow_depth.vert", "assets/shaders/engine/shadow/shadow_depth.frag");
        shader_library_.load("point_shadow_depth", "assets/shaders/engine/shadow/point_shadow_depth.vert", "assets/shaders/engine/shadow/point_shadow_depth.frag", "assets/shaders/engine/shadow/point_shadow_depth.geom");

        shader_library_.load_compute("occlusion_cull", "assets/shaders/engine/occlusion/occlusion_cull.comp");
        shader_library_.load_compute("hzb_generate", "assets/shaders/engine/occlusion/hzb_generate.comp");

        depth_prepass_fbo_ = std::make_unique<Framebuffer>(framebuffer_width_, framebuffer_height_, FramebufferType::DepthOnly);

        shadow_framebuffer_ = std::make_unique<Framebuffer>(4096, 4096, FramebufferType::ShadowMap);
        point_shadow_framebuffers_.clear();
        for (size_t i = 0; i < MAX_POINT_LIGHT_SHADOWS; i++) {
            point_shadow_framebuffers_.push_back(std::make_unique<Framebuffer>(1024, 1024, FramebufferType::PointShadowMap));
        }

        instance_vbo_capacity_bytes_ = 1000 * sizeof(InstanceData);
        instance_vbo_ = std::make_shared<VertexBuffer>(nullptr, instance_vbo_capacity_bytes_, true);

        CORE_INFO("Renderer successfully initialized");
        return true;
    }

    void Renderer::on_detach() {
        if (post_processor_) {
            post_processor_->shutdown();
            post_processor_.reset();
        }
        depth_prepass_fbo_.reset();
        shadow_framebuffer_.reset();
        point_shadow_framebuffers_.clear();
        camera_ubo_.reset();
    }

    void Renderer::on_resize(const int width, const int height) {
        framebuffer_width_ = width;
        framebuffer_height_ = height;
        RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        if (depth_prepass_fbo_) {
            depth_prepass_fbo_->resize(width, height);
        }

        if (post_processor_) {
            post_processor_->resize(width, height);
        }
    }

    void Renderer::render() {
        // Final render code here
        if (should_render_ && post_processor_) {
            post_processor_->render_to_screen();
        }
    }

    void Renderer::bind_camera(const Camera& camera) {
        current_frustum_ = camera.get_frustum();
        const CameraData camera_data{
            .view       = camera.get_view_matrix(),
            .projection = camera.get_projection_matrix()
        };
        camera_ubo_->set_data(&camera_data, sizeof(CameraData));
    }

    void Renderer::begin_scene() {
        reset_stats();
        clear_lights();
        if (post_processor_) {
            post_processor_->begin_scene();
        }
    }

    void Renderer::end_scene() {
        if (post_processor_) {
            post_processor_->end_scene();
        }
    }

    void Renderer::render_occlusion_pass(Camera& camera, AssetManager& asset_manager, std::unordered_map<UUID, std::vector<InstanceData>>& batches) {
        if (!depth_prepass_fbo_) return;

        auto depth_shader = shader_library_.get("depth_prepass");
        if (!depth_shader) return;

        // 1. Depth Pre-Pass
        depth_prepass_fbo_->bind();
        RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(framebuffer_width_), static_cast<uint32_t>(framebuffer_height_));
        RenderCommand::set_depth_test(true);
        RenderCommand::set_depth_mask(true);

        // Color Clear-Value auf 1.0 (Fernste Tiefe)
        RenderCommand::set_clear_color(glm::vec4(1.0f));
        RenderCommand::clear(true, true);

        const bool prev_culling = is_frustum_culling_enabled();
        set_frustum_culling_enabled(false);

        for (const auto& [handle, instance_list] : batches) {
            if (instance_list.empty()) continue;
            if (std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle)) {
                draw_mesh_instanced(*mesh, instance_list, depth_shader);
            }
        }

        set_frustum_culling_enabled(prev_culling);
        depth_prepass_fbo_->unbind();

        // 2. Build HZB Depth Pyramid Mipmaps
        auto hzb_shader = shader_library_.get("hzb_generate");
        if (!hzb_shader) return;

        hzb_shader->bind();
        depth_prepass_fbo_->bind_color_texture(0);

        uint32_t current_width = framebuffer_width_;
        uint32_t current_height = framebuffer_height_;
        const uint32_t total_mips = depth_prepass_fbo_->get_mip_levels();

        for (uint32_t mip = 1; mip < total_mips; ++mip) {
            uint32_t next_width = std::max(1u, current_width / 2);
            uint32_t next_height = std::max(1u, current_height / 2);

            depth_prepass_fbo_->bind_color_image(1, mip, true);

            hzb_shader->set_int("u_src_mip_level", static_cast<int>(mip - 1));
            hzb_shader->set_ivec2("u_out_resolution", glm::ivec2(next_width, next_height));

            const uint32_t groups_x = (next_width + 15) / 16;
            const uint32_t groups_y = (next_height + 15) / 16;

            RenderCommand::dispatch_compute(groups_x, groups_y, 1);

            current_width = next_width;
            current_height = next_height;
        }

        // 3. Occlusion Compute Pass
        auto occlusion_shader = shader_library_.get("occlusion_cull");
        if (!occlusion_shader) return;

        struct InstanceRef {
            UUID handle;
            size_t index;
        };

        std::vector<InstanceAABB> instance_aabbs;
        std::vector<InstanceRef> instance_refs;

        for (const auto& [handle, instance_list] : batches) {
            std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle);
            if (!mesh) continue;

            for (size_t i = 0; i < instance_list.size(); ++i) {
                const AABB world_aabb = mesh->local_bounds.get_transformed(instance_list[i].transform);
                instance_aabbs.push_back(InstanceAABB{
                    .min_pos = glm::vec4(world_aabb.min, 1.0f),
                    .max_pos = glm::vec4(world_aabb.max, 1.0f)
                });
                instance_refs.push_back({handle, i});
            }
        }

        if (instance_aabbs.empty()) return;

        const auto num_instances = static_cast<uint32_t>(instance_aabbs.size());

        StorageBuffer aabb_ssbo(instance_aabbs.data(), instance_aabbs.size() * sizeof(InstanceAABB), 0);
        StorageBuffer visibility_ssbo(num_instances * sizeof(uint32_t), 1);

        aabb_ssbo.bind(0);
        visibility_ssbo.bind(1);

        occlusion_shader->bind();
        depth_prepass_fbo_->bind_color_texture(0);

        const glm::mat4 vp = camera.get_projection_matrix() * camera.get_view_matrix();
        occlusion_shader->set_mat4("u_view_projection", vp);
        occlusion_shader->set_vec2("u_screen_size", glm::vec2(framebuffer_width_, framebuffer_height_));
        occlusion_shader->set_int("u_num_instances", num_instances);
        occlusion_shader->set_int("u_hzb_texture", 0);

        RenderCommand::dispatch_compute((num_instances + 63) / 64, 1, 1);

        std::vector<uint32_t> visibility_results(num_instances);
        visibility_ssbo.get_data(visibility_results.data(), visibility_results.size() * sizeof(uint32_t));

        aabb_ssbo.unbind();
        visibility_ssbo.unbind();
        RenderCommand::unbind_texture(0);
        RenderCommand::unbind_image(1);

        std::unordered_map<UUID, std::vector<InstanceData>> filtered_batches;

        uint32_t visible_count = 0;
        for (size_t i = 0; i < num_instances; ++i) {
            if (visibility_results[i] == 1) {
                const auto& [handle, index] = instance_refs[i];
                filtered_batches[handle].push_back(batches[handle][index]);
                visible_count++;
            }
        }

        CORE_INFO("Occlusion Culling: Input={0}, Output={1}", num_instances, visible_count);

        batches = std::move(filtered_batches);
    }

    void Renderer::render_shadow_pass(Camera& camera, AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches) {
        if (directional_lights_.empty() || !shadow_framebuffer_) return;

        const glm::vec3 main_light_dir = directional_lights_[0].direction;
        const glm::vec3 shadow_center = camera.get_target();

        constexpr float shadow_map_size = 4096.0f;
        // Todo: CSM (Cascaded shadow maps)
        const float ortho_size = std::clamp(camera.get_distance() * 0.5f, 15.0f, /*max distance*/ 100.0f);
        const float light_distance = ortho_size * 2.0f;

        const glm::vec3 light_pos = shadow_center - main_light_dir * light_distance;
        const glm::vec3 up = std::abs(main_light_dir.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

        const glm::mat4 light_view = glm::lookAt(light_pos, shadow_center, up);
        glm::mat4 light_projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, 0.1f, light_distance * 2.0f);

        // Texel Snapping
        const glm::mat4 shadow_matrix = light_projection * light_view;
        glm::vec4 shadow_origin = shadow_matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        shadow_origin *= (shadow_map_size / 2.0f);

        const glm::vec4 rounded_origin = glm::round(shadow_origin);
        glm::vec4 round_offset = rounded_origin - shadow_origin;
        round_offset /= (shadow_map_size / 2.0f);
        round_offset.z = 0.0f;
        round_offset.w = 0.0f;

        light_projection[3] += round_offset;
        light_space_matrix_ = light_projection * light_view;

        const Frustum light_frustum = Frustum::from_view_projection(light_space_matrix_);

        shadow_framebuffer_->bind();
        RenderCommand::clear(false, true);

        auto shadow_shader = shader_library_.get("shadow_depth");
        if (shadow_shader) {
            shadow_shader->bind();
            shadow_shader->set_mat4("u_light_space_matrix", light_space_matrix_);

            RenderCommand::set_polygon_offset(true, 3.0f, 6.0f);

            const bool prev_culling = is_frustum_culling_enabled();
            set_frustum_culling_enabled(false);

            for (const auto& [handle, instance_list] : batches) {
                std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle);
                if (!mesh || instance_list.empty()) continue;

                std::vector<InstanceData> culled_instances;
                culled_instances.reserve(instance_list.size());

                // Light-Space culling
                for (const auto& instance : instance_list) {
                    const AABB world_aabb = mesh->local_bounds.get_transformed(instance.transform);
                    if (light_frustum.intersects(world_aabb)) {
                        culled_instances.push_back(instance);
                    }
                }

                if (!culled_instances.empty()) {
                    draw_mesh_instanced(*mesh, culled_instances, shadow_shader);
                }
            }
            set_frustum_culling_enabled(prev_culling);
            RenderCommand::set_polygon_offset(false);
        }

        shadow_framebuffer_->unbind();
        RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(framebuffer_width_), static_cast<uint32_t>(framebuffer_height_));
    }

    void Renderer::render_point_shadow_pass(AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches) {
        if (point_lights_.empty() || point_shadow_framebuffers_.empty()) return;

        auto point_shadow_shader = shader_library_.get("point_shadow_depth");
        if (!point_shadow_shader) return;

        RenderCommand::set_depth_test(true);
        RenderCommand::set_depth_mask(true);
        RenderCommand::set_polygon_offset(true, 1.0f, 2.0f);
        RenderCommand::set_cull_face(true, true);

        const bool prev_culling = is_frustum_culling_enabled();
        set_frustum_culling_enabled(false);

        const size_t shadow_lights_count = std::min(point_lights_.size(), MAX_POINT_LIGHT_SHADOWS);

        for (size_t light_index = 0; light_index < shadow_lights_count; ++light_index) {
            const auto& light = point_lights_[light_index];
            const glm::vec3 light_pos = light.position;
            const float far_plane = light.radius;

            const glm::mat4 shadow_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, far_plane);

            const glm::mat4 shadow_matrices[6] = {
                shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
            };

            auto& fbo = point_shadow_framebuffers_[light_index];
            fbo->bind();
            RenderCommand::set_viewport(0, 0, fbo->get_width(), fbo->get_height());

            point_shadow_shader->bind();
            point_shadow_shader->set_vec3("u_light_position", light_pos);
            point_shadow_shader->set_float("u_far_plane", far_plane);

            for (uint32_t face = 0; face < 6; ++face) {
                point_shadow_shader->set_mat4("u_shadow_matrices[" + std::to_string(face) + "]", shadow_matrices[face]);
            }
            RenderCommand::clear(false, true);

            for (const auto& [handle, instance_list] : batches) {
                std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle);
                if (!mesh || instance_list.empty()) continue;

                std::vector<InstanceData> culled_instances;
                culled_instances.reserve(instance_list.size());

                for (const auto& instance : instance_list) {
                    const AABB world_aabb = mesh->local_bounds.get_transformed(instance.transform);
                    if (world_aabb.intersects_sphere(light_pos, far_plane)) {
                        culled_instances.push_back(instance);
                    }
                }

                if (!culled_instances.empty()) {
                    draw_mesh_instanced(*mesh, culled_instances, point_shadow_shader);
                }
            }
            fbo->unbind();
        }

        set_frustum_culling_enabled(prev_culling);
        RenderCommand::set_cull_face(false);
        RenderCommand::set_polygon_offset(false);

        RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(framebuffer_width_), static_cast<uint32_t>(framebuffer_height_));
    }

    void Renderer::render_mesh_batches(AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches) {
        if (post_processor_) {
            post_processor_->begin_scene();
            RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(framebuffer_width_), static_cast<uint32_t>(framebuffer_height_));
        }

        RenderCommand::set_depth_test(true);
        RenderCommand::set_depth_mask(true);

        RenderCommand::set_cull_face(false);

        for (const auto& [handle, instance_list] : batches) {
            if (std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle)) {
                draw_mesh_instanced(*mesh, instance_list);
            }
        }
    }

    void Renderer::set_blend(const bool enable) {
        RenderCommand::set_blend(enable);
    }

    void Renderer::draw_arrays(const VertexArray& vao, const uint32_t count) {
        RenderCommand::draw_arrays(vao, count);
    }

    void Renderer::set_clear_color(const glm::vec4& color) {
        RenderCommand::set_clear_color(color);
    }

    void Renderer::clear(const glm::vec4& color) {
        RenderCommand::set_clear_color(color);
        RenderCommand::clear(true, true);
    }

    void Renderer::reset_buffers() {
        // Reset instance buffer capacity back to default (1000 instances)
        instance_vbo_capacity_bytes_ = 1000 * sizeof(InstanceData);
        instance_vbo_ = std::make_shared<VertexBuffer>(nullptr, instance_vbo_capacity_bytes_, true);
    }

    Mesh Renderer::upload_mesh(const std::vector<Vertex>& vertices_in, const std::vector<uint32_t>& indices) {
        if (vertices_in.empty() || indices.empty()) {
            CORE_WARN("Renderer: Cannot upload empty mesh");
            return {};
        }

        std::vector<Vertex> vertices = vertices_in;

        // Auto-generate tangents if unassigned
        if (glm::length(vertices[0].tangent) < 0.0001f) {
            for (size_t k = 0; k < indices.size(); k += 3) {
                Vertex& v0 = vertices[indices[k]];
                Vertex& v1 = vertices[indices[k + 1]];
                Vertex& v2 = vertices[indices[k + 2]];

                glm::vec3 edge1 = v1.position - v0.position;
                glm::vec3 edge2 = v2.position - v0.position;
                glm::vec2 deltaUV1 = v1.uv - v0.uv;
                glm::vec2 deltaUV2 = v2.uv - v0.uv;

                float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                float f = (std::abs(denom) < 0.0001f) ? 1.0f : 1.0f / denom;

                glm::vec3 tangent{
                    f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                    f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                    f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
                };

                v0.tangent += tangent;
                v1.tangent += tangent;
                v2.tangent += tangent;
            }

            for (auto& v : vertices) {
                v.tangent = (glm::length(v.tangent) > 0.0001f) ? glm::normalize(v.tangent) : glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }

        AABB local_bounds{
            glm::vec3(std::numeric_limits<float>::max()),
            glm::vec3(std::numeric_limits<float>::lowest())
        };
        for (const auto& vertex : vertices) {
            local_bounds.min = glm::min(local_bounds.min, vertex.position);
            local_bounds.max = glm::max(local_bounds.max, vertex.position);
        }
        auto vao = std::make_shared<VertexArray>();

        const auto vbo = std::make_shared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
        const BufferLayout layout = {
            {ShaderDataType::Float3, "a_position"},
            {ShaderDataType::Float3, "a_normal"},
            {ShaderDataType::Float2, "a_uv"},
            {ShaderDataType::Float3, "a_tangent"}
        };
        vao->add_vertex_buffer(vbo, layout);

        const auto ebo = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
        vao->set_index_buffer(ebo);

        Mesh mesh{};
        mesh.vertex_array = std::move(vao);
        mesh.local_bounds = local_bounds;
        return mesh;
    }

    void Renderer::destroy_mesh(Mesh& mesh) {
        mesh.vertex_array.reset();
    }

    void Renderer::draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix, const glm::vec4& color) {
        draw_mesh_instanced(mesh, {{.transform = model_matrix, .color = color}});
    }

    void Renderer::draw_mesh_instanced(const Mesh& mesh, const std::vector<InstanceData>& instances, const std::shared_ptr<Shader>& override_shader) {
        if (instances.empty() || !mesh.is_valid()) return;

        // const std::vector<InstanceData>* instances_to_draw = &instances;
        // std::vector<InstanceData> visible_instances;

        // if (frustum_culling_enabled_) {
        //     visible_instances.reserve(instances.size());
        //     for (const auto& instance : instances) {
        //         const AABB world_aabb = mesh.local_bounds.get_transformed(instance.transform);
        //         if (current_frustum_.intersects(world_aabb)) {
        //             visible_instances.push_back(instance);
        //         }
        //     }
        //
        //     if (visible_instances.empty()) return;
        //     instances_to_draw = &visible_instances;
        // }

        stats_.draw_calls++;
        stats_.instance_count += static_cast<uint32_t>(instances.size());

        const auto shader = override_shader ? override_shader : shader_library_.get("mesh");
        if (!shader) return;

        const size_t bytes_needed = instances.size() * sizeof(InstanceData);

        // Dynamic VBO resizing
        if (bytes_needed > instance_vbo_capacity_bytes_) {
            instance_vbo_capacity_bytes_ = bytes_needed * 2;
            instance_vbo_ = std::make_shared<VertexBuffer>(nullptr, instance_vbo_capacity_bytes_, true);
        }

        // Upload raw batch directly to GPU
        instance_vbo_->set_data(instances.data(), bytes_needed);

        shader->bind();

        if (!override_shader) {
            // Albedo Map (Slot 0)
            if (mesh.texture) {
                mesh.texture->bind(0);
                shader->set_int("u_albedo_texture", 0);
                shader->set_int("u_use_albedo_texture", 1);
            }
            else {
                shader->set_int("u_use_albedo_texture", 0);
            }

            // Normal Map (Slot 1)
            if (mesh.normal_map) {
                mesh.normal_map->bind(1);
                shader->set_int("u_normal_texture", 1);
                shader->set_int("u_use_normal_texture", 1);
            }
            else {
                shader->set_int("u_use_normal_texture", 0);
            }

            // Metallic-Roughness Map (Slot 2)
            if (mesh.metallic_roughness_map) {
                mesh.metallic_roughness_map->bind(2);
                shader->set_int("u_metallic_roughness_texture", 2);
                shader->set_int("u_use_metallic_roughness_texture", 1);
            }
            else {
                shader->set_int("u_use_metallic_roughness_texture", 0);
            }

            // Directional Shadow Map (Slot 3)
            if (shadow_framebuffer_) {
                shadow_framebuffer_->bind_depth_texture(3);
                shader->set_int("u_shadow_map", 3);
                shader->set_mat4("u_light_space_matrix", light_space_matrix_);
            }

            // Point Shadow Maps (Slots 4 - 7)
            for (size_t i = 0; i < MAX_POINT_LIGHT_SHADOWS; ++i) {
                const uint32_t slot = 4 + static_cast<uint32_t>(i);
                if (i < point_shadow_framebuffers_.size()) {
                    point_shadow_framebuffers_[i]->bind_depth_texture(slot);
                    shader->set_int("u_point_shadow_maps[" + std::to_string(i) + "]", static_cast<int>(slot));
                }
            }

            // Lights
            shader->set_int("u_num_dir_lights", static_cast<int>(directional_lights_.size()));
            for (size_t i = 0; i < directional_lights_.size(); ++i) {
                std::string base = "u_dir_lights[" + std::to_string(i) + "].";
                shader->set_vec3(base + "direction", directional_lights_[i].direction);
                shader->set_vec3(base + "color", directional_lights_[i].color);
                shader->set_float(base + "intensity", directional_lights_[i].intensity);
            }

            shader->set_int("u_num_point_lights", static_cast<int>(point_lights_.size()));
            for (size_t i = 0; i < point_lights_.size(); ++i) {
                std::string base = "u_point_lights[" + std::to_string(i) + "].";
                shader->set_vec3(base + "position", point_lights_[i].position);
                shader->set_vec3(base + "color", point_lights_[i].color);
                shader->set_float(base + "intensity", point_lights_[i].intensity);
                shader->set_float(base + "radius", point_lights_[i].radius);
            }
        }

        RenderCommand::draw_indexed_instanced(
            *mesh.vertex_array,
            *instance_vbo_,
            mesh.vertex_array->get_index_buffer()->get_count(),
            static_cast<uint32_t>(instances.size())
        );

        shader->unbind();
    }

    void Renderer::set_frustum_culling_enabled(bool enabled) {
        frustum_culling_enabled_ = enabled;
    }

    bool Renderer::is_frustum_culling_enabled() const {
        return frustum_culling_enabled_;
    }
}
