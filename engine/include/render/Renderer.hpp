//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <memory>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <render/Mesh.hpp>
#include <render/Vertex.hpp>

namespace engine {
    class Camera;

    struct LineVertex {
        glm::vec3 position;
        glm::vec3 color;

        LineVertex(const glm::vec3& pos, const glm::vec3& col)
            : position(pos), color(col) {}
    };

    class Renderer {
    public:
        Renderer();
        ~Renderer();

        bool init(GLFWwindow* window);

        void shutdown();

        void resize(int width, int height);

        // -------------------------
        // Frames
        // -------------------------
        void begin_frame();
        void end_frame(float delta_time);

        // -------------------------
        // Mesh
        // -------------------------
        /** @brief Uploads generic vertex and index buffers to the GPU */
        [[nodiscard]] static Mesh upload_mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        static void destroy_mesh(Mesh& mesh);

        /** @brief Draws an uploaded Mesh using a world transform matrix */
        void draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix) const;
        // -------------------------
        // Rendering
        // -------------------------
        void draw_line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
        void draw_box(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
        void draw_circle(const glm::vec3& center, float radius, const glm::vec3& normal, const glm::vec3& color, int segments = 32);
        void draw_sphere(const glm::vec3& center, float radius, const glm::vec3& color, int segments = 32);

        /** @brief Draws all lines at the end of the frame */
        void flush_lines();

        // -------------------------
        // Debug controls
        // -------------------------
        void set_grid_enabled(const bool toggle) { show_grid_ = toggle; }
        [[nodiscard]] bool is_grid_enabled() const { return show_grid_; }

        void set_gizmo_enabled(const bool toggle) { show_gizmo_ = toggle; }
        [[nodiscard]] bool is_gizmo_enabled() const { return show_gizmo_; }

        [[nodiscard]] bool is_debug_panel_enabled() const { return show_debug_panel_; }
        void set_debug_panel_enabled(const bool enabled) { show_debug_panel_ = enabled; }
        void toggle_debug_panel() { show_debug_panel_ = !show_debug_panel_; }
        // -------------------------

        [[nodiscard]] Camera& get_camera() const;

    private:
        static unsigned int build_shader_program();

        GLFWwindow* window_ = nullptr;
        unsigned int shader_program_ = 0;

        int framebuffer_width_ = 1;
        int framebuffer_height_ = 1;

        std::unique_ptr<Camera> camera_ = nullptr;

        // -----------------------------------------
        // Editor overlays
        // -----------------------------------------
        void render_editor_overlays();

        bool show_grid_ = true;
        bool show_gizmo_ = true;
        bool show_debug_panel_ = true;

        void render_grid() const;
        void render_gizmo();

        // -----------------------------------------
        // Line rendering
        // -----------------------------------------
        std::vector<LineVertex> line_batch_buffer_;

        size_t line_vbo_capacity_bytes_ = 0;

        unsigned int line_shader_ = 0;
        unsigned int line_vao_ = 0;
        unsigned int line_vbo_ = 0;
        int line_u_view_loc_ = -1;
        int line_u_proj_loc_ = -1;

        unsigned int grid_shader_ = 0;
        unsigned int grid_vao_ = 0;
        int grid_u_view_loc_ = -1;
        int grid_u_proj_loc_ = -1;

        void init_line_resources();
        void init_grid_resources();

        void cleanup_line_resources();
        void cleanup_grid_resources();

    };
}
