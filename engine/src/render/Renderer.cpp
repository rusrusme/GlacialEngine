//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "render/Renderer.hpp"

#include "asset/AssetManager.hpp"
#include "core/Config.hpp"
#include "core/Framebuffer.hpp"
#include "render/Camera.hpp"
#include "render/Texture2D.hpp"
#include "render/VertexArray.hpp"
#include "render/mesh/InstanceData.hpp"

namespace engine {
    Renderer::Renderer() = default;
    Renderer::~Renderer() = default;

    bool Renderer::on_init(GLFWwindow* window) {
        window_ = window;

        if (window_ == nullptr) {
            std::cerr << "Engine error: renderer: GLFWwindow pointer is null" << std::endl;
            return false;
        }

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            std::cerr << "Engine error: renderer: failed to initialize GLAD" << std::endl;
            return false;
        }

        glfwGetFramebufferSize(window_, &framebuffer_width_, &framebuffer_height_);
        glViewport(0, 0, framebuffer_width_, framebuffer_height_);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // Todo: Check if needed

        camera_ubo_ = std::make_unique<UniformBuffer>(sizeof(CameraData), 0);

        shader_library_.load("mesh", "assets/shaders/engine/mesh.vert", "assets/shaders/engine/mesh.frag");
        shader_library_.load("line", "assets/shaders/engine/line.vert", "assets/shaders/engine/line.frag");
        shader_library_.load("shadow_depth", "assets/shaders/engine/shadow/shadow_depth.vert", "assets/shaders/engine/shadow/shadow_depth.frag");
        shader_library_.load("point_shadow_depth", "assets/shaders/engine/shadow/point_shadow_depth.vert", "assets/shaders/engine/shadow/point_shadow_depth.frag");

        shadow_framebuffer_ = std::make_unique<Framebuffer>(4096, 4096, FramebufferType::ShadowMap);
        point_shadow_framebuffer_ = std::make_unique<Framebuffer>(1024, 1024, FramebufferType::PointShadowMap);

        instance_vbo_capacity_bytes_ = 1000 * sizeof(InstanceData);
        instance_vbo_ = std::make_shared<VertexBuffer>(nullptr, instance_vbo_capacity_bytes_, true);

        init_line_resources();
        return true;
    }

    void Renderer::on_shutdown() {
        shadow_framebuffer_.reset();
        line_batch_buffer_.clear();
        line_vao_.reset();
        line_vbo_.reset();
        camera_ubo_.reset();
    }

    void Renderer::on_resize(const int width, const int height) {
        framebuffer_width_ = width;
        framebuffer_height_ = height;
        glViewport(0, 0, framebuffer_width_, framebuffer_height_);
    }

    void Renderer::begin_scene(const Camera& camera) {
        reset_stats();
        clear_lights();

        current_frustum_ = camera.get_frustum();

        const CameraData camera_data{
            .view       = camera.get_view_matrix(),
            .projection = camera.get_projection_matrix()
        };
        camera_ubo_->set_data(&camera_data, sizeof(CameraData));
    }

    void Renderer::end_scene(const float delta_time) {
        flush();
    }

    void Renderer::render_shadow_pass(Camera& camera, AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches) {
        if (directional_lights_.empty() || !shadow_framebuffer_) return;

        const glm::vec3 main_light_dir = directional_lights_[0].direction;
        const glm::vec3 shadow_center = camera.get_target();

        constexpr float shadow_map_size = 4096.0f;
        const float ortho_size = std::clamp(camera.get_distance() * 0.5f, 15.0f, 60.0f);
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

        // Save active framebuffer and viewport
        GLint prev_fbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
        GLint prev_viewport[4];
        glGetIntegerv(GL_VIEWPORT, prev_viewport);

        shadow_framebuffer_->bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        auto shadow_shader = shader_library_.get("shadow_depth");
        if (shadow_shader) {
            shadow_shader->bind();
            shadow_shader->set_mat4("u_light_space_matrix", light_space_matrix_);

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(3.0f, 6.0f);

            const bool prev_culling = is_frustum_culling_enabled();
            set_frustum_culling_enabled(false);

            for (const auto& [handle, instance_list] : batches) {
                if (std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle)) {
                    draw_mesh_instanced(*mesh, instance_list, shadow_shader);
                }
            }
            set_frustum_culling_enabled(prev_culling);
            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        // Restore active framebuffer and viewport
        glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
        glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    }

    void Renderer::render_point_shadow_pass(AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches) {
        if (point_lights_.empty() || !point_shadow_framebuffer_) return;

        const auto& light = point_lights_[0];
        const glm::vec3 light_pos = light.position;
        const float far_plane = light.radius;

        const glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, far_plane);

        // Standard OpenGL Cubemap Cardinal Orientations
        const std::vector<glm::mat4> shadow_transforms = {
            shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // +X
            shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // -X
            shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),   // +Y
            shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), // -Y
            shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // +Z
            shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))  // -Z
        };

        GLint prev_fbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
        GLint prev_viewport[4];
        glGetIntegerv(GL_VIEWPORT, prev_viewport);

        point_shadow_framebuffer_->bind();

        // 1. Force viewport to shadow map texture dimensions
        glViewport(0, 0, point_shadow_framebuffer_->get_width(), point_shadow_framebuffer_->get_height());

        // 2. Hardware Rasterizer Depth Bias & Front-Face Culling
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        // 3. Disable main camera frustum culling
        const bool prev_culling = is_frustum_culling_enabled();
        set_frustum_culling_enabled(false);

        auto point_shadow_shader = shader_library_.get("point_shadow_depth");
        if (point_shadow_shader) {
            point_shadow_shader->bind();
            point_shadow_shader->set_vec3("u_light_position", light_pos);
            point_shadow_shader->set_float("u_far_plane", far_plane);

            for (uint32_t face = 0; face < 6; ++face) {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                    point_shadow_framebuffer_->get_depth_attachment_id(),
                    0
                );
                glClear(GL_DEPTH_BUFFER_BIT);

                point_shadow_shader->set_mat4("u_light_space_matrix", shadow_transforms[face]);

                for (const auto& [handle, instance_list] : batches) {
                    if (std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle)) {
                        draw_mesh_instanced(*mesh, instance_list, point_shadow_shader);
                    }
                }
            }
        }

        // Restore state
        set_frustum_culling_enabled(prev_culling);
        glCullFace(GL_BACK);
        glDisable(GL_POLYGON_OFFSET_FILL);

        glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
        glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    }

    void Renderer::render_mesh_batches(AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches) {
        for (const auto& [handle, instance_list] : batches) {
            if (std::shared_ptr<Mesh> mesh = asset_manager.get_asset<Mesh>(handle)) {
                draw_mesh_instanced(*mesh, instance_list);
            }
        }
    }

    void Renderer::set_blend(const bool enable) {
        if (enable) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else {
            glDisable(GL_BLEND);
        }
    }

    void Renderer::draw_arrays(const VertexArray& vao, const uint32_t count) {
        vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(count));
        vao.unbind();
    }

    void Renderer::set_clear_color(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void Renderer::clear(const glm::vec4& color) {
        set_clear_color(color);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    Mesh Renderer::upload_mesh(const std::vector<Vertex>& vertices_in, const std::vector<uint32_t>& indices) {
        if (vertices_in.empty() || indices.empty()) {
            std::cerr << "Engine error: renderer: cannot upload empty mesh" << std::endl;
            return {};
        }

        std::vector<Vertex> vertices = vertices_in;

        // Auto-generate tangents if unassigned (e.g. primitive meshes)
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

    void Renderer::draw_mesh_instanced(const Mesh& mesh, const std::vector<InstanceData>& instances, const std::shared_ptr<Shader>& override_shader) {
        if (instances.empty() || !mesh.is_valid()) return;

        const std::vector<InstanceData>* instances_to_draw = &instances;
        std::vector<InstanceData> visible_instances;

        if (frustum_culling_enabled_) {
            visible_instances.reserve(instances.size());
            for (const auto& instance : instances) {
                if (current_frustum_.intersects(mesh.local_bounds, instance.transform)) {
                    visible_instances.push_back(instance);
                }
            }

            if (visible_instances.empty()) return; // All instances culled
            instances_to_draw = &visible_instances;
        }

        stats_.draw_calls++;
        stats_.instance_count += static_cast<uint32_t>(instances_to_draw->size());

        const auto shader = override_shader ? override_shader : shader_library_.get("mesh");
        if (!shader) return;

        const size_t bytes_needed = instances_to_draw->size() * sizeof(InstanceData);

        if (bytes_needed > instance_vbo_capacity_bytes_) {
            instance_vbo_capacity_bytes_ = bytes_needed * 2;
            instance_vbo_ = std::make_shared<VertexBuffer>(nullptr, instance_vbo_capacity_bytes_, true);
        }
        instance_vbo_->set_data(instances_to_draw->data(), bytes_needed);

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

            // Shadow Map (Slot 3)
            if (shadow_framebuffer_) {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, shadow_framebuffer_->get_depth_attachment_id());
                shader->set_int("u_shadow_map", 3);
                shader->set_mat4("u_light_space_matrix", light_space_matrix_);
            }
            // Point Shadow Map (Slot 4)
            if (point_shadow_framebuffer_) {
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_framebuffer_->get_depth_attachment_id());
                shader->set_int("u_point_shadow_map", 4);
            }

            // Directional & Point Lights
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

        mesh.vertex_array->bind();
        instance_vbo_->bind();

        constexpr auto stride = static_cast<GLsizei>(sizeof(InstanceData));
        for (uint32_t i = 0; i < 4; ++i) {
            glEnableVertexAttribArray(4 + i);
            glVertexAttribPointer(
                4 + i,
                4,
                GL_FLOAT,
                GL_FALSE,
                stride,
                reinterpret_cast<const void*>(sizeof(glm::vec4) * i)
            );
            glVertexAttribDivisor(4 + i, 1);
        }

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(
            8,
            4,
            GL_FLOAT,
            GL_FALSE,
            stride,
            reinterpret_cast<const void*>(offsetof(InstanceData, color))
        );
        glVertexAttribDivisor(8, 1);

        glEnableVertexAttribArray(9);
        glVertexAttribPointer(
            9,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            reinterpret_cast<const void*>(offsetof(InstanceData, pbr))
        );
        glVertexAttribDivisor(9, 1);

        glDrawElementsInstanced(
            GL_TRIANGLES,
            static_cast<GLsizei>(mesh.vertex_array->get_index_buffer()->get_count()),
            GL_UNSIGNED_INT,
            nullptr,
            static_cast<GLsizei>(instances_to_draw->size())
        );

        mesh.vertex_array->unbind();
        shader->unbind();
    }

    void Renderer::draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix, const glm::vec4& color) {
        draw_mesh_instanced(mesh, {{.transform = model_matrix, .color = color}});
    }

    void Renderer::draw_line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
        line_batch_buffer_.emplace_back(from, color);
        line_batch_buffer_.emplace_back(to, color);
    }

    void Renderer::draw_box(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color) {
        glm::vec3 p1 = min;
        glm::vec3 p2 = {max.x, min.y, min.z};
        glm::vec3 p3 = {max.x, min.y, max.z};
        glm::vec3 p4 = {min.x, min.y, max.z};

        glm::vec3 p5 = {min.x, max.y, min.z};
        glm::vec3 p6 = {max.x, max.y, min.z};
        glm::vec3 p7 = max;
        glm::vec3 p8 = {min.x, max.y, max.z};

        draw_line(p1, p2, color);
        draw_line(p2, p3, color);
        draw_line(p3, p4, color);
        draw_line(p4, p1, color);

        draw_line(p5, p6, color);
        draw_line(p6, p7, color);
        draw_line(p7, p8, color);
        draw_line(p8, p5, color);

        draw_line(p1, p5, color);
        draw_line(p2, p6, color);
        draw_line(p3, p7, color);
        draw_line(p4, p8, color);
    }

    void Renderer::draw_circle(const glm::vec3& center, const float radius, const glm::vec3& normal, const glm::vec3& color, const int segments) {
        const glm::vec3 n = glm::normalize(normal);

        const glm::vec3 helper = std::abs(n.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
        const glm::vec3 u = glm::normalize(glm::cross(helper, n));
        const glm::vec3 v = glm::cross(n, u);

        const float angle_step = glm::two_pi<float>() / static_cast<float>(segments);

        for (int i = 0; i < segments; ++i) {
            const float a1 = static_cast<float>(i) * angle_step;
            const float a2 = static_cast<float>(i + 1) * angle_step;

            glm::vec3 p1 = center + (u * std::cos(a1) + v * std::sin(a1)) * radius;
            glm::vec3 p2 = center + (u * std::cos(a2) + v * std::sin(a2)) * radius;

            draw_line(p1, p2, color);
        }
    }

    void Renderer::draw_sphere(const glm::vec3& center, const float radius, const glm::vec3& color, const int segments) {
        draw_circle(center, radius, {1, 0, 0}, color, segments);
        draw_circle(center, radius, {0, 1, 0}, color, segments);
        draw_circle(center, radius, {0, 0, 1}, color, segments);
    }

    void Renderer::flush() {
        auto line_shader = shader_library_.get("line");
        if (line_batch_buffer_.empty() || !line_shader || !line_vao_) return;

        const size_t bytes_needed = line_batch_buffer_.size() * sizeof(LineVertex);

        if (bytes_needed > line_vbo_capacity_bytes_) {
            line_vbo_capacity_bytes_ = bytes_needed * 2;
            line_vbo_ = std::make_shared<VertexBuffer>(nullptr, line_vbo_capacity_bytes_, true);

            line_vao_ = std::make_unique<VertexArray>();
            const BufferLayout line_layout = {
                {ShaderDataType::Float3, "a_position"},
                {ShaderDataType::Float3, "a_color"}
            };
            line_vao_->add_vertex_buffer(line_vbo_, line_layout);
        }

        line_vbo_->set_data(line_batch_buffer_.data(), bytes_needed);

        line_shader->bind();
        line_vao_->bind();

        glDepthMask(GL_FALSE);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(line_batch_buffer_.size()));
        glDepthMask(GL_TRUE);

        line_vao_->unbind();
        line_shader->unbind();

        line_batch_buffer_.clear();
    }

    void Renderer::init_line_resources() {
        line_vbo_capacity_bytes_ = 2000 * sizeof(LineVertex);
        line_vbo_ = std::make_shared<VertexBuffer>(nullptr, line_vbo_capacity_bytes_, true);

        line_vao_ = std::make_unique<VertexArray>();
        const BufferLayout line_layout = {
            {ShaderDataType::Float3, "a_position"},
            {ShaderDataType::Float3, "a_color"}
        };
        line_vao_->add_vertex_buffer(line_vbo_, line_layout);

        line_batch_buffer_.reserve(2000);
    }
}
