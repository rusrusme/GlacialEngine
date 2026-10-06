//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/debug/DebugRenderer.hpp"

#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>

#include "render/legacy/Camera.hpp"
#include "render/legacy/RenderCommand.hpp"

namespace engine::legacy {
    using namespace engine;
    void DebugRenderer::on_attach() {
        data_.line_vbo_capacity_bytes = 2000 * sizeof(LineVertex);
        data_.line_vbo = std::make_shared<VertexBuffer>(nullptr, data_.line_vbo_capacity_bytes, true);

        data_.line_vao = std::make_unique<VertexArray>();
        const BufferLayout line_layout = {
            {ShaderDataType::Float3, "a_position"},
            {ShaderDataType::Float3, "a_color"}
        };
        data_.line_vao->add_vertex_buffer(data_.line_vbo, line_layout);

        data_.line_shader = GLShader::from_file(
            "assets/shaders/legacy/engine/debug/line.vert",
            "assets/shaders/legacy/engine/debug/line.frag"
        );
        data_.line_batch_buffer.reserve(2000);
    }

    void DebugRenderer::on_detach() {
        data_.line_batch_buffer.clear();
        data_.line_vao.reset();
        data_.line_vbo.reset();
        data_.line_shader.reset();
    }

    void DebugRenderer::begin_scene(const Camera& camera) {
        // Empty
    }

    void DebugRenderer::end_scene() {
        flush();
    }

    void DebugRenderer::draw_line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
        data_.line_batch_buffer.emplace_back(from, color);
        data_.line_batch_buffer.emplace_back(to, color);
    }

    void DebugRenderer::draw_box(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color) {
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

    void DebugRenderer::draw_circle(const glm::vec3& center, const float radius, const glm::vec3& normal, const glm::vec3& color, const int segments) {
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

    void DebugRenderer::draw_sphere(const glm::vec3& center, const float radius, const glm::vec3& color, const int segments) {
        draw_circle(center, radius, {1, 0, 0}, color, segments);
        draw_circle(center, radius, {0, 1, 0}, color, segments);
        draw_circle(center, radius, {0, 0, 1}, color, segments);
    }

    void DebugRenderer::flush() {
        if (data_.line_batch_buffer.empty() || !data_.line_shader || !data_.line_vao) return;

        const size_t bytes_needed = data_.line_batch_buffer.size() * sizeof(LineVertex);

        if (bytes_needed > data_.line_vbo_capacity_bytes) {
            data_.line_vbo_capacity_bytes = bytes_needed * 2;
            data_.line_vbo = std::make_shared<VertexBuffer>(nullptr, data_.line_vbo_capacity_bytes, true);

            data_.line_vao = std::make_unique<VertexArray>();
            const BufferLayout line_layout = {
                {ShaderDataType::Float3, "a_position"},
                {ShaderDataType::Float3, "a_color"}
            };
            data_.line_vao->add_vertex_buffer(data_.line_vbo, line_layout);
        }

        data_.line_vbo->set_data(data_.line_batch_buffer.data(), bytes_needed);

        data_.line_shader->bind();

        RenderCommand::set_depth_mask(false);
        RenderCommand::draw_lines(*data_.line_vao, static_cast<uint32_t>(data_.line_batch_buffer.size()));
        RenderCommand::set_depth_mask(true);

        data_.line_shader->unbind();
        data_.line_batch_buffer.clear();
    }
}
