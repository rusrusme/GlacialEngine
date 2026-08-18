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
#include "ShaderLibrary.hpp"
#include "UniformBuffer.hpp"

namespace engine {
    class Camera;

    struct LineVertex {
        glm::vec3 position;
        glm::vec3 color;

        LineVertex(const glm::vec3& pos, const glm::vec3& col) : position(pos), color(col) {}
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

        /** @brief Enables or disables blending for rendering */
        void set_blend(bool enable) const;

        /** @brief Draws a VertexArray with a specified number of vertices. This is useful for rendering simple shapes or primitives without the need for an index buffer. */
        void draw_arrays(const VertexArray& vao, uint32_t count) const;

        // -------------------------
        // Mesh
        // -------------------------
        [[nodiscard]] static Mesh upload_mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        static void destroy_mesh(Mesh& mesh);
        void draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix) const;

        // -------------------------
        // Debug line rendering
        // -------------------------
        void draw_line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
        void draw_box(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
        void draw_circle(const glm::vec3& center, float radius, const glm::vec3& normal, const glm::vec3& color, int segments = 32);
        void draw_sphere(const glm::vec3& center, float radius, const glm::vec3& color, int segments = 32);

        /** @brief Draws all lines at the end of the frame */
        void flush();

        // -------------------------
        // Asset access
        // -------------------------
        [[nodiscard]] ShaderLibrary& get_shader_library() { return shader_library_; }
        [[nodiscard]] const ShaderLibrary& get_shader_library() const { return shader_library_; }
        // -------------------------

        struct CameraData {
            glm::mat4 view;
            glm::mat4 projection;
        };

    private:
        ShaderLibrary shader_library_;
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

        void init_line_resources();
    };
}
