//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "render/Renderer.hpp"

#include "core/Config.hpp"
#include "render/Camera.hpp"
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

        camera_ubo_ = std::make_unique<UniformBuffer>(sizeof(CameraData), 0);

        shader_library_.load("mesh", "assets/shaders/mesh.vert", "assets/shaders/mesh.frag");
        shader_library_.load("line", "assets/shaders/line.vert", "assets/shaders/line.frag");

        instance_vbo_capacity_bytes_ = 1000 * sizeof(InstanceData);
        instance_vbo_ = std::make_shared<VertexBuffer>(nullptr, instance_vbo_capacity_bytes_, true);

        init_line_resources();
        return true;
    }

    void Renderer::on_shutdown() {
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

        const CameraData camera_data{
            .view       = camera.get_view_matrix(),
            .projection = camera.get_projection_matrix()
        };
        camera_ubo_->set_data(&camera_data, sizeof(CameraData));
    }

    void Renderer::end_scene(const float delta_time) {
        flush();
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

    void Renderer::draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix) {
        draw_mesh(mesh, model_matrix, {1.0f, 1.0f, 1.0f, 1.0f});
    }

    void Renderer::draw_mesh_instanced(const Mesh& mesh, const std::vector<InstanceData>& instances) {
        if (instances.empty() || !mesh.is_valid()) return;

        stats_.draw_calls++;
        stats_.instance_count += static_cast<uint32_t>(instances.size());

        const auto shader = shader_library_.get("mesh");
        if (!shader) return;

        const size_t bytes_needed = instances.size() * sizeof(InstanceData);

        if (bytes_needed > instance_vbo_capacity_bytes_) {
            instance_vbo_capacity_bytes_ = bytes_needed * 2;
            instance_vbo_ = std::make_shared<VertexBuffer>(nullptr, instance_vbo_capacity_bytes_, true);
        }
        instance_vbo_->set_data(instances.data(), bytes_needed);

        shader->bind();

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

        // Bind Directional and Point Lights
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
        mesh.vertex_array->bind();
        instance_vbo_->bind();

        const auto stride = static_cast<GLsizei>(sizeof(InstanceData));
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

        glDrawElementsInstanced(
        GL_TRIANGLES,
        static_cast<GLsizei>(mesh.vertex_array->get_index_buffer()->get_count()),
        GL_UNSIGNED_INT,
        nullptr,
        static_cast<GLsizei>(instances.size())
    );

        mesh.vertex_array->unbind();
        shader->unbind();
    }

    void Renderer::draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix, const glm::vec4& color) {
        draw_mesh_instanced(mesh, {{model_matrix, color}});
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

        // Falls das Line-Buffer-Capacity überschritten wird, vergrößern wir den VertexBuffer
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
