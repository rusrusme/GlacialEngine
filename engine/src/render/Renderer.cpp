//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "render/Renderer.hpp"

#include <imgui.h>
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

    bool Renderer::init(GLFWwindow* window) {
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

        camera_ = std::make_unique<Camera>();
        camera_->apply_default_view();

        return true;
    }

    void Renderer::shutdown() {
        camera_ = nullptr;

        if (shader_program_ != 0) {
            glDeleteProgram(shader_program_);
            shader_program_ = 0;
        }
    }

    void Renderer::resize(const int width, const int height) {
        framebuffer_width_ = width;
        framebuffer_height_ = height;
        glViewport(0, 0, framebuffer_width_, framebuffer_height_);
    }

    void Renderer::begin_frame() {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::draw() {
        if (show_grid_) {
            render_grid();
        }
        if (show_gizmo_) {
            render_gizmo();
        }

        // ----------------------
        // Debug panel
        // ----------------------
        if (show_debug_panel_) {
            ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
            ImGui::Begin("Debug panel");
            const float fps = ImGui::GetIO().Framerate;
            const float frame_time_ms = 1000.0f / fps;

            ImGui::Text("Performance");
            ImGui::Text("  FPS: %.1f", fps);
            ImGui::Text("  Frame Time: %.2f ms", frame_time_ms);

            ImGui::Separator();
            ImGui::Text("Video");
            static bool vsync_enabled = false;
            if (ImGui::Checkbox("V-Sync", &vsync_enabled)) {
                glfwSwapInterval(vsync_enabled ? 1 : 0);
            }
            ImGui::Text("FPS Limit: %s", setting::get_target_fps_str().c_str());
            ImGui::SliderInt("##", &setting::target_fps_index, 0, setting::num_presets - 1, setting::get_target_fps_str().c_str());

            ImGui::Separator();
            ImGui::Text("Editor");
            bool grid_active = is_grid_enabled();
            if (ImGui::Checkbox("Show World Grid", &grid_active)) {
                set_grid_enabled(grid_active);
            }

            bool gizmo_active = is_gizmo_enabled();
            if (ImGui::Checkbox("Show Axis Gizmo", &gizmo_active)) {
                set_gizmo_enabled(gizmo_active);
            }
            ImGui::End();
        }
    }

    void Renderer::end_frame() {
    }

    Mesh Renderer::upload_mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        if (vertices.empty() || indices.empty()) {
            std::cerr << "Engine error: renderer: cannot upload empty mesh" << std::endl;
            return {};
        }

        Mesh mesh{};
        mesh.index_count = static_cast<uint32_t>(indices.size());

        // Todo: OpenGL magic

        return mesh;
    }

    void Renderer::destroy_mesh(Mesh& mesh) {
        if (mesh.ebo != 0) {
            glDeleteBuffers(1, &mesh.ebo);
            mesh.ebo = 0;
        }
        if (mesh.vbo != 0) {
            glDeleteBuffers(1, &mesh.vbo);
            mesh.vbo = 0;
        }
        if (mesh.vao != 0) {
            glDeleteVertexArrays(1, &mesh.vao);
            mesh.vao = 0;
        }
        mesh.index_count = 0;
    }

    void Renderer::draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix) const {
        if (shader_program_ == 0 || mesh.vao == 0 || mesh.index_count == 0 || camera_ == nullptr) {
            return;
        }
        glUseProgram(shader_program_);

        const glm::mat4 view = camera_->get_view_matrix();
        const glm::mat4 projection = camera_->get_projection_matrix();

        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "u_model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.index_count), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    void Renderer::draw_line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) const {
        if (camera_ == nullptr) return;

        static unsigned int line_shader = 0;
        static int u_view = -1, u_proj = -1, u_color = -1;
        if (line_shader == 0) {
            constexpr auto vsrc = R"(
                #version 450 core
                layout(location = 0) in vec3 a_position;
                uniform mat4 u_view;
                uniform mat4 u_projection;
                void main() {
                    gl_Position = u_projection * u_view * vec4(a_position, 1.0);
                }
            )";
            constexpr auto fsrc = R"(
                #version 450 core
                uniform vec3 u_color;
                out vec4 frag_color;
                void main() {
                    frag_color = vec4(u_color, 1.0);
                }
            )";
            const unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, &vsrc, nullptr);
            glCompileShader(vs);
            const unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, &fsrc, nullptr);
            glCompileShader(fs);
            line_shader = glCreateProgram();
            glAttachShader(line_shader, vs);
            glAttachShader(line_shader, fs);
            glLinkProgram(line_shader);
            glDeleteShader(vs);
            glDeleteShader(fs);
            u_view = glGetUniformLocation(line_shader, "u_view");
            u_proj = glGetUniformLocation(line_shader, "u_projection");
            u_color = glGetUniformLocation(line_shader, "u_color");
        }
        const float vertices[6] = {
            from.x, from.y, from.z,
            to.x, to.y, to.z
        };
        unsigned int vbo = 0, vao = 0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glUseProgram(line_shader);
        glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(camera_->get_view_matrix()));
        glUniformMatrix4fv(u_proj, 1, GL_FALSE, glm::value_ptr(camera_->get_projection_matrix()));

        glUniform3fv(u_color, 1, &color[0]);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    Camera& Renderer::get_camera() const {
        return *camera_;
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
        if (camera_ == nullptr) return;

        static unsigned int grid_shader = 0;
        static unsigned int dummy_vao = 0;
        static int u_view = -1, u_proj = -1;

        if (grid_shader == 0) {
            const unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, &grid_vertex_shader, nullptr);
            glCompileShader(vs);

            const unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, &grid_fragment_shader, nullptr);
            glCompileShader(fs);

            grid_shader = glCreateProgram();
            glAttachShader(grid_shader, vs);
            glAttachShader(grid_shader, fs);
            glLinkProgram(grid_shader);

            glDeleteShader(vs);
            glDeleteShader(fs);

            u_view = glGetUniformLocation(grid_shader, "u_view");
            u_proj = glGetUniformLocation(grid_shader, "u_projection");

            // Dummy VAO required by Core Profile when rendering with gl_VertexID
            glGenVertexArrays(1, &dummy_vao);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(grid_shader);
        glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(camera_->get_view_matrix()));
        glUniformMatrix4fv(u_proj, 1, GL_FALSE, glm::value_ptr(camera_->get_projection_matrix()));

        glBindVertexArray(dummy_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw fullscreen quad (6 vertices)
        glBindVertexArray(0);

        glDisable(GL_BLEND);
    }

    void Renderer::render_gizmo() const {
        // World Origin Gizmo (X = Red, Y = Green, Z = Blue)
        draw_line({0, 0, 0}, {2, 0, 0}, {1, 0, 0});
        draw_line({0, 0, 0}, {0, 2, 0}, {0, 1, 0});
        draw_line({0, 0, 0}, {0, 0, 2}, {0, 0, 1});
    }
}
