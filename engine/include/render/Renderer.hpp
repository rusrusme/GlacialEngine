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
        void draw();
        void end_frame();

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
        void draw_line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) const;

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

        bool show_grid_ = true;
        bool show_gizmo_ = true;
        bool show_debug_panel_ = true;

        void render_grid() const;
        void render_gizmo() const;
    };
}
