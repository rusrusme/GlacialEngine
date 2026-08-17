//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <render/Mesh.hpp>
#include <render/Vertex.hpp>

#include "Shader.hpp"
#include "UniformBuffer.hpp"

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

        bool on_init(GLFWwindow* window);
        void on_shutdown();
        void on_resize(int width, int height);

        // -------------------------
        // Scene
        // -------------------------
        void begin_scene(const Camera& camera);
        void end_scene(float delta_time);

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
        void flush();

        [[deprecated]] void render_grid() const;
        // -------------------------

        struct CameraData {
            glm::mat4 view;
            glm::mat4 projection;
        };

    private:
        std::unique_ptr<Shader> mesh_shader_;
        std::unique_ptr<Shader> line_shader_;
        std::unique_ptr<Shader> grid_shader_;

        std::unique_ptr<UniformBuffer> camera_ubo_;

        GLFWwindow* window_ = nullptr;

        int framebuffer_width_ = 1;
        int framebuffer_height_ = 1;

        // -----------------------------------------
        // Line & Grid Rendering Resources
        // -----------------------------------------
        std::vector<LineVertex> line_batch_buffer_;
        size_t line_vbo_capacity_bytes_ = 0;

        std::unique_ptr<VertexArray> line_vao_;
        std::shared_ptr<VertexBuffer> line_vbo_;

        std::unique_ptr<VertexArray> grid_vao_;

        void init_line_resources();
        void init_grid_resources();
    };
}
