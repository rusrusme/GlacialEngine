//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <unordered_map>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "core/UUID.hpp"
#include "render/Shader.hpp"
#include "render/ShaderLibrary.hpp"
#include "render/UniformBuffer.hpp"
#include "render/Vertex.hpp"
#include "render/mesh/Mesh.hpp"

namespace engine {
    class Framebuffer;
    class AssetManager;
    class VertexBuffer;
    struct InstanceData;
    class Camera;

    struct LineVertex {
        glm::vec3 position;
        glm::vec3 color;

        LineVertex(const glm::vec3& pos, const glm::vec3& col) : position(pos), color(col) {}
    };

    struct RenderStats {
        uint32_t draw_calls = 0;
        uint32_t instance_count = 0;
    };

    class Renderer {
    public:
        struct DirectionalLightData;
        struct PointLightData;

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

        void render_shadow_pass(Camera& camera, AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches);

        /** @brief Renders all opaque mesh batches */
        void render_mesh_batches(AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches);

        /** @brief Enables or disables blending for rendering */
        static void set_blend(bool enable);

        /** @brief Draws a VertexArray with a specified number of vertices. This is useful for rendering simple shapes or primitives without the need for an index buffer. */
        static void draw_arrays(const VertexArray& vao, uint32_t count);

        static void set_clear_color(const glm::vec4& color);
        static void clear(const glm::vec4& color = {0.12f, 0.12f, 0.12f, 1.0f});

        // -------------------------
        // Mesh
        // -------------------------
        [[nodiscard]] static Mesh upload_mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        static void destroy_mesh(Mesh& mesh);
        void draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix, const glm::vec4& color);
        void draw_mesh_instanced(const Mesh& mesh, const std::vector<InstanceData>& instances, const std::shared_ptr<Shader>& override_shader = nullptr);

        // Frustum culling
        void set_frustum_culling_enabled(bool enabled) { frustum_culling_enabled_ = enabled; }
        [[nodiscard]] bool is_frustum_culling_enabled() const { return frustum_culling_enabled_; }

        // Debug line rendering
        void draw_line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
        void draw_box(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
        void draw_circle(const glm::vec3& center, float radius, const glm::vec3& normal, const glm::vec3& color, int segments = 32);
        void draw_sphere(const glm::vec3& center, float radius, const glm::vec3& color, int segments = 32);
        void flush();

        // Asset access
        [[nodiscard]] ShaderLibrary& get_shader_library() { return shader_library_; }
        [[nodiscard]] const ShaderLibrary& get_shader_library() const { return shader_library_; }

        // Light
        void add_directional_light(const DirectionalLightData& light) {
            if (directional_lights_.size() < 4) directional_lights_.push_back(light);
        }

        void add_point_light(const PointLightData& light) {
            if (point_lights_.size() < 8) point_lights_.push_back(light);
        }

        void clear_lights() {
            directional_lights_.clear();
            point_lights_.clear();
        }

        struct CameraData {
            glm::mat4 view;
            glm::mat4 projection;
        };

        struct DirectionalLightData {
            glm::vec3 direction{0.0f, -1.0f, 0.0f};
            glm::vec3 color{0.0f, 0.0f, 0.0f};
            float intensity{0.0f};
        };

        struct PointLightData {
            glm::vec3 position{0.0f, 0.0f, 0.0f};
            glm::vec3 color{1.0f, 1.0f, 1.0f};
            float intensity{1.0f};
            float radius{10.0f};
        };

        [[nodiscard]] int get_framebuffer_width() const { return framebuffer_width_; }
        [[nodiscard]] int get_framebuffer_height() const { return framebuffer_height_; }
        [[nodiscard]] const RenderStats& get_stats() const { return stats_; }
        void reset_stats() { stats_ = RenderStats{}; }

    private:
        RenderStats stats_;

        ShaderLibrary shader_library_;
        std::unique_ptr<UniformBuffer> camera_ubo_;

        std::vector<DirectionalLightData> directional_lights_;
        std::vector<PointLightData> point_lights_;

        GLFWwindow* window_ = nullptr;

        int framebuffer_width_ = 1;
        int framebuffer_height_ = 1;

        // Frustum culling
        Frustum current_frustum_;
        bool frustum_culling_enabled_{true};

        // Shadow resources
        std::unique_ptr<Framebuffer> shadow_framebuffer_;
        glm::mat4 light_space_matrix_{1.0f};

        // Line
        std::vector<LineVertex> line_batch_buffer_;
        size_t line_vbo_capacity_bytes_ = 0;

        std::unique_ptr<VertexArray> line_vao_;
        std::shared_ptr<VertexBuffer> line_vbo_;

        // Instanced
        std::shared_ptr<VertexBuffer> instance_vbo_;
        size_t instance_vbo_capacity_bytes_ = 0;

        void init_line_resources();
    };
}
