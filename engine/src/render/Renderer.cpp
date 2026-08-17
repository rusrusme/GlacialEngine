//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "render/Renderer.hpp"

#ifdef ENABLE_IMGUI
#include <imgui.h>
#endif

#include <iostream>
#include <memory>
#include <core/Config.hpp>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <render/Camera.hpp>

namespace {
    constexpr auto grid_vertex_shader = R"(
        #version 450 core

        // Fullscreen quad positions in Normalized Device Coordinates (NDC)
        vec3 positions[6] = vec3[](
            vec3(-1.0, -1.0, 0.0),
            vec3( 1.0, -1.0, 0.0),
            vec3( 1.0,  1.0, 0.0),
            vec3(-1.0, -1.0, 0.0),
            vec3( 1.0,  1.0, 0.0),
            vec3(-1.0,  1.0, 0.0)
        );

        uniform mat4 u_view;
        uniform mat4 u_projection;

        out vec3 v_near_point;
        out vec3 v_far_point;

        // Unprojects 2D screen coordinates back into 3D world space
        vec3 unproject_point(float x, float y, float z, mat4 view_proj_inv) {
            vec4 unprojected = view_proj_inv * vec4(x, y, z, 1.0);
            return unprojected.xyz / unprojected.w;
        }

        void main() {
            vec3 pos = positions[gl_VertexID];
            mat4 inv_view_proj = inverse(u_projection * u_view);

            // Unproject screen point on the near plane (z = 0.0) and far plane (z = 1.0)
            v_near_point = unproject_point(pos.x, pos.y, 0.0, inv_view_proj);
            v_far_point  = unproject_point(pos.x, pos.y, 1.0, inv_view_proj);

            gl_Position = vec4(pos, 1.0); // Output fullscreen quad directly
        }
    )";

    constexpr auto grid_fragment_shader = R"(
        #version 450 core
        in vec3 v_near_point;
        in vec3 v_far_point;

        out vec4 frag_color;

        uniform mat4 u_view;
        uniform mat4 u_projection;

        float draw_grid(vec2 pt, float spacing) {
            vec2 coord = pt / spacing;
            vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
            float line = min(grid.x, grid.y);
            return 1.0 - min(line, 1.0);
        }

        void main() {
            // Calculate ray direction from near to far unprojected points
            vec3 ray_dir = v_far_point - v_near_point;

            // Solve ray-plane intersection for Y = 0:
            // v_near_point.y + t * ray_dir.y = 0  =>  t = -v_near_point.y / ray_dir.y
            float t = -v_near_point.y / ray_dir.y;

            // Discard pixels looking away from the floor plane (t < 0)
            if (t <= 0.0) discard;

            // Exact 3D world coordinate where camera ray hits Y = 0
            vec3 world_pos = v_near_point + t * ray_dir;

            // Write correct depth value to OpenGL depth buffer so objects obscure the grid properly
            vec4 clip_space_pos = u_projection * u_view * vec4(world_pos, 1.0);
            gl_FragDepth = (clip_space_pos.z / clip_space_pos.w) * 0.5 + 0.5;

            // Draw procedural grid
            vec2 coord = world_pos.xz;
            float grid1 = draw_grid(coord, 1.0);
            float grid10 = draw_grid(coord, 10.0);

            // Smooth distance fade out
            float dist = length(world_pos - v_near_point);
            float fade = max(0.0, 1.0 - (dist / 1000.0)); // Fades smoothly

            vec4 color = vec4(0.35, 0.35, 0.35, grid1 * 0.5);
            color = mix(color, vec4(0.6, 0.6, 0.6, 0.8), grid10);

            if (color.a < 0.01) discard;

            frag_color = vec4(color.rgb, color.a * fade);
        }
    )";
}

namespace {
    constexpr auto vertex_shader_source = R"(
        #version 450 core
        layout (location = 0) in vec3 a_position;
        layout (location = 1) in vec3 a_normal;
        layout (location = 2) in vec2 a_uv;

        uniform mat4 u_model;
        uniform mat4 u_view;
        uniform mat4 u_projection;

        out vec3 v_normal;

        void main() {
            v_normal = mat3(transpose(inverse(u_model))) * a_normal;
            gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
        }
    )";

    constexpr auto fragment_shader_source = R"(
        #version 450 core
        out vec4 frag_color;

        void main() {
            frag_color = vec4(1.0);
        }
    )";
}

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

        shader_program_ = build_shader_program();
        if (shader_program_ == 0) {
            return false;
        }

        init_line_resources();
        init_grid_resources();

        return true;
    }

    void Renderer::on_shutdown() {
        cleanup_line_resources();
        cleanup_grid_resources();

        if (shader_program_ != 0) {
            glDeleteProgram(shader_program_);
            shader_program_ = 0;
        }
    }

    void Renderer::on_resize(const int width, const int height) {
        framebuffer_width_ = width;
        framebuffer_height_ = height;
        glViewport(0, 0, framebuffer_width_, framebuffer_height_);
    }

    void Renderer::begin_scene(const Camera& camera) {
        projection_matrix_ = camera.get_projection_matrix();
        view_matrix_ = camera.get_view_matrix();
    }

    void Renderer::end_scene(const float delta_time) {
        flush();
    }

    Mesh Renderer::upload_mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        if (vertices.empty() || indices.empty()) {
            std::cerr << "Engine error: renderer: cannot upload empty mesh" << std::endl;
            return {};
        }

        auto vao = std::make_shared<VertexArray>();

        // 1. VertexBuffer erstellen & Layout passend zu struct Vertex definieren
        auto vbo = std::make_shared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
        BufferLayout layout = {
            {ShaderDataType::Float3, "a_position"},
            {ShaderDataType::Float3, "a_normal"},
            {ShaderDataType::Float2, "a_uv"}
        };
        vao->add_vertex_buffer(vbo, layout);

        // 2. IndexBuffer erstellen
        auto ebo = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
        vao->set_index_buffer(ebo);

        Mesh mesh{};
        mesh.vertex_array = std::move(vao);
        return mesh;
    }

    void Renderer::destroy_mesh(Mesh& mesh) {
        mesh.vertex_array.reset();
    }

    void Renderer::draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix) const {
        if (!mesh.is_valid() || shader_program_ == 0) {
            return;
        }

        // (Später ersetzen wir shader_program_ durch dein Shader-Objekt)
        glUseProgram(shader_program_);
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "u_model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "u_view"), 1, GL_FALSE, glm::value_ptr(view_matrix_));
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix_));

        mesh.vertex_array->bind();
        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(mesh.vertex_array->get_index_buffer()->get_count()),
            GL_UNSIGNED_INT,
            nullptr
        );
        mesh.vertex_array->unbind();
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
        if (line_batch_buffer_.empty()) return;

        const size_t bytes_needed = line_batch_buffer_.size() * sizeof(LineVertex);

        glBindVertexArray(line_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);

        if (bytes_needed > line_vbo_capacity_bytes_) {
            line_vbo_capacity_bytes_ = bytes_needed * 2;
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(line_vbo_capacity_bytes_), nullptr, GL_DYNAMIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(bytes_needed), line_batch_buffer_.data());
        }
        else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(bytes_needed), line_batch_buffer_.data());
        }

        glUseProgram(line_shader_);
        glUniformMatrix4fv(line_u_view_loc_, 1, GL_FALSE, glm::value_ptr(view_matrix_));
        glUniformMatrix4fv(line_u_proj_loc_, 1, GL_FALSE, glm::value_ptr(projection_matrix_));

        glDepthMask(GL_FALSE);

        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(line_batch_buffer_.size()));

        glDepthMask(GL_TRUE);
        glBindVertexArray(0);

        line_batch_buffer_.clear();
    }

    unsigned int Renderer::build_shader_program() {
        const unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
        glCompileShader(vertex_shader);

        int success = 0;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
            std::cerr << "Engine error: renderer: vertex shader compilation failed\n" << info_log << std::endl;
            glDeleteShader(vertex_shader);
            return 0;
        }

        const unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
        glCompileShader(fragment_shader);

        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
            std::cerr << "Engine error: renderer: fragment shader compilation failed\n" << info_log << std::endl;
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            return 0;
        }

        const unsigned int program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(program, 512, nullptr, info_log);
            std::cerr << "Engine error: renderer: shader program linking failed\n" << info_log << std::endl;
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            glDeleteProgram(program);
            return 0;
        }

        glDetachShader(program, vertex_shader);
        glDetachShader(program, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return program;
    }

    void Renderer::render_grid() const {
        if (grid_shader_ == 0) return;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(grid_shader_);
        glUniformMatrix4fv(grid_u_view_loc_, 1, GL_FALSE, glm::value_ptr(view_matrix_));
        glUniformMatrix4fv(grid_u_proj_loc_, 1, GL_FALSE, glm::value_ptr(projection_matrix_));

        glBindVertexArray(grid_vao_);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw fullscreen quad (6 vertices)
        glBindVertexArray(0);

        glDisable(GL_BLEND);
    }

    void Renderer::init_line_resources() {
        constexpr auto line_vsrc = R"(
            #version 450 core
            layout(location = 0) in vec3 a_position;
            layout(location = 1) in vec3 a_color;

            uniform mat4 u_view;
            uniform mat4 u_projection;

            out vec3 v_color;

            void main() {
                v_color = a_color;
                gl_Position = u_projection * u_view * vec4(a_position, 1.0);
            }
        )";

        constexpr auto line_fsrc = R"(
        #version 450 core
        in vec3 v_color;
        out vec4 frag_color;

        void main() {
            frag_color = vec4(v_color, 1.0);
        }
        )";

        const unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &line_vsrc, nullptr);
        glCompileShader(vs);

        const unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &line_fsrc, nullptr);
        glCompileShader(fs);

        line_shader_ = glCreateProgram();
        glAttachShader(line_shader_, vs);
        glAttachShader(line_shader_, fs);
        glLinkProgram(line_shader_);

        glDeleteShader(vs);
        glDeleteShader(fs);

        line_u_view_loc_ = glGetUniformLocation(line_shader_, "u_view");
        line_u_proj_loc_ = glGetUniformLocation(line_shader_, "u_projection");

        glGenVertexArrays(1, &line_vao_);
        glGenBuffers(1, &line_vbo_);

        glBindVertexArray(line_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);

        line_vbo_capacity_bytes_ = 2000 * sizeof(LineVertex);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(line_vbo_capacity_bytes_), nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), reinterpret_cast<void*>(offsetof(LineVertex, position)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), reinterpret_cast<void*>(offsetof(LineVertex, color)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        line_batch_buffer_.reserve(2000);
    }

    void Renderer::init_grid_resources() {
        const unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &grid_vertex_shader, nullptr);
        glCompileShader(vs);

        const unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &grid_fragment_shader, nullptr);
        glCompileShader(fs);

        grid_shader_ = glCreateProgram();
        glAttachShader(grid_shader_, vs);
        glAttachShader(grid_shader_, fs);
        glLinkProgram(grid_shader_);

        glDeleteShader(vs);
        glDeleteShader(fs);

        grid_u_view_loc_ = glGetUniformLocation(grid_shader_, "u_view");
        grid_u_proj_loc_ = glGetUniformLocation(grid_shader_, "u_projection");

        glGenVertexArrays(1, &grid_vao_);
    }

    void Renderer::cleanup_line_resources() {
        if (line_vbo_ != 0) {
            glDeleteBuffers(1, &line_vbo_);
            line_vbo_ = 0;
        }
        if (line_vao_ != 0) {
            glDeleteVertexArrays(1, &line_vao_);
            line_vao_ = 0;
        }
        if (line_shader_ != 0) {
            glDeleteProgram(line_shader_);
            line_shader_ = 0;
        }
    }

    void Renderer::cleanup_grid_resources() {
        if (grid_vao_ != 0) {
            glDeleteVertexArrays(1, &grid_vao_);
            grid_vao_ = 0;
        }
        if (grid_shader_ != 0) {
            glDeleteProgram(grid_shader_);
            grid_shader_ = 0;
        }
    }
}
