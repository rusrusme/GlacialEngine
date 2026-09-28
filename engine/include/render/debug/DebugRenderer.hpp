//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/Renderer.hpp"

namespace engine {
    class Camera;

    struct LineVertex {
        glm::vec3 position;
        glm::vec3 color;

        LineVertex(const glm::vec3& pos, const glm::vec3& col) : position(pos), color(col) {}
    };

    struct DebugData {
        std::shared_ptr<Shader> line_shader;
        std::unique_ptr<VertexArray> line_vao;
        std::shared_ptr<VertexBuffer> line_vbo;

        std::vector<LineVertex> line_batch_buffer;
        size_t line_vbo_capacity_bytes = 0;
    };

    class DebugRenderer {
    public:
        static void on_attach();
        static void on_detach();

        static void begin_scene(const Camera& camera);
        static void end_scene();

        static void draw_line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
        static void draw_box(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
        static void draw_circle(const glm::vec3& center, float radius, const glm::vec3& normal, const glm::vec3& color, int segments = 32);
        static void draw_sphere(const glm::vec3& center, float radius, const glm::vec3& color, int segments = 32);

    private:
        static void flush();

        inline static DebugData data_;
    };
}
