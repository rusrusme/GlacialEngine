//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

#include "core/UUID.hpp"
#include "render/Buffer.hpp"
#include "render/Framebuffer.hpp"
#include "render/Frustum.hpp"
#include "render/PostProcessor.hpp"
#include "render/Shader.hpp"
#include "render/ShaderLibrary.hpp"
#include "render/UniformBuffer.hpp"
#include "render/Vertex.hpp"
#include "render/mesh/Mesh.hpp"

namespace engine {
    class AssetManager;
    struct InstanceData;
    class Camera;

    struct RenderStats {
        uint32_t draw_calls = 0;
        uint32_t instance_count = 0;
    };

    struct InstanceAABB {
        glm::vec4 min_pos{0.0f}; // xyz = min bounds
        glm::vec4 max_pos{0.0f}; // xyz = max bounds
    };

    class Renderer {
    public:
        struct DirectionalLightData;
        struct PointLightData;

        Renderer();
        ~Renderer();

        bool on_attach(void* window);
        void on_detach();
        void on_resize(int width, int height);

        void render();
        void bind_camera(const Camera& camera);

        // Scene
        void begin_scene();
        void end_scene();

        void render_occlusion_pass(Camera& camera, AssetManager& asset_manager, std::unordered_map<UUID, std::vector<InstanceData>>& batches);
        void render_shadow_pass(Camera& camera, AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches);
        void render_point_shadow_pass(AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches);

        /** @brief Renders all opaque mesh batches */
        void render_mesh_batches(AssetManager& asset_manager, const std::unordered_map<UUID, std::vector<InstanceData>>& batches);

        /** @brief Enables or disables blending for rendering */
        static void set_blend(bool enable);

        /** @brief Draws a VertexArray with a specified number of vertices. This is useful for rendering simple shapes or primitives without the need for an index buffer. */
        static void draw_arrays(const VertexArray& vao, uint32_t count);

        static void set_clear_color(const glm::vec4& color);
        static void clear(const glm::vec4& color = {0.12f, 0.12f, 0.12f, 1.0f});
        void reset_buffers();

        // Mesh
        [[nodiscard]] static Mesh upload_mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        static void destroy_mesh(Mesh& mesh);
        void draw_mesh(const Mesh& mesh, const glm::mat4& model_matrix, const glm::vec4& color);
        void draw_mesh_instanced(const Mesh& mesh, const std::vector<InstanceData>& instances, const std::shared_ptr<Shader>& override_shader = nullptr);

        // Frustum culling
        void set_frustum_culling_enabled(bool enabled);
        [[nodiscard]] bool is_frustum_culling_enabled() const;

        // Asset access
        [[nodiscard]] ShaderLibrary& get_shader_library() { return shader_library_; }

        std::shared_ptr<Shader> load_shader(const std::string& name, const std::string& bsh_path) {
            return shader_library_.load(name, bsh_path);
        }

        std::shared_ptr<Shader> load_shader(const std::string& name,
                                            const std::string& vertex_path,
                                            const std::string& fragment_path,
                                            const std::string& geometry_path = "") {
            return shader_library_.load(name, vertex_path, fragment_path, geometry_path);
        }

        [[nodiscard]] std::shared_ptr<Shader> get_shader(const std::string& name) { return shader_library_.get(name); }
        [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<Shader>>& get_shaders() { return shader_library_.get_shaders(); }

        // Light
        static constexpr size_t MAX_POINT_LIGHTS = 8;
        static constexpr size_t MAX_POINT_LIGHT_SHADOWS = 4;

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

        struct CameraData {
            glm::mat4 view;
            glm::mat4 projection;
        };

        [[nodiscard]] int get_framebuffer_width() const { return framebuffer_width_; }
        [[nodiscard]] int get_framebuffer_height() const { return framebuffer_height_; }
        [[nodiscard]] const RenderStats& get_stats() const { return stats_; }
        void reset_stats() { stats_ = RenderStats{}; }

        // Post processing
        [[nodiscard]] PostProcessSettings& get_post_process_settings() { return post_processor_->get_settings(); }

        // Texture handle for ImGui Editor Viewports
        [[nodiscard]] uint32_t get_final_color_texture() const { return post_processor_->get_final_color_texture(); }

        void set_render_to_screen(const bool enable) { should_render_ = enable; }
        [[nodiscard]] bool should_render_to_screen() const { return should_render_; }

    private:
        RenderStats stats_;

        ShaderLibrary shader_library_;
        std::unique_ptr<UniformBuffer> camera_ubo_;

        std::vector<DirectionalLightData> directional_lights_;
        std::vector<PointLightData> point_lights_;

        void* window_ = nullptr;

        int framebuffer_width_ = 1;
        int framebuffer_height_ = 1;

        // Frustum culling
        Frustum current_frustum_;
        bool frustum_culling_enabled_{true};

        // Occlusion resources
        std::unique_ptr<Framebuffer> depth_prepass_fbo_;

        // Shadow resources
        std::unique_ptr<Framebuffer> shadow_framebuffer_;
        [[deprecated]] std::unique_ptr<Framebuffer> point_shadow_framebuffer_;
        std::vector<std::unique_ptr<Framebuffer>> point_shadow_framebuffers_;
        glm::mat4 light_space_matrix_{1.0f};

        // Instanced
        std::shared_ptr<VertexBuffer> instance_vbo_;
        size_t instance_vbo_capacity_bytes_ = 0;

        // Post processing
        std::unique_ptr<PostProcessor> post_processor_;
        bool should_render_ = true;
    };
}
