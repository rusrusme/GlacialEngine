//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

#include "render/RenderObject.hpp"
#include "render/camera/Camera.hpp"
#include "render/pass/MainOpaquePass.hpp"
#include "render/rhi/CommandBuffer.hpp"
#include "render/rhi/Framebuffer.hpp"
#include "render/rhi/RHIDevice.hpp"
#include "render/rhi/Texture.hpp"

namespace engine {
    struct FrameUniformData;
    class Scene;
    struct SystemContext;

    class SceneRenderer {
    public:
        SceneRenderer() = default;
        ~SceneRenderer() = default;

        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer& operator=(const SceneRenderer&) = delete;
        SceneRenderer(SceneRenderer&&) noexcept = delete;
        SceneRenderer& operator=(SceneRenderer&&) noexcept = delete;

        void init(const RHIDevice& device);
        void shutdown();

        void begin_scene(const FrameUniformData& frame_data);
        void end_scene(CommandBuffer& cb, Framebuffer& target_framebuffer);

        void submit(const RenderObject& object);
        void submit(const Mesh* mesh, const glm::mat4& transform, const Texture2D* albedo_texture = nullptr, const glm::vec4& albedo_color = glm::vec4(1.0f));

        void render_scene(const Shared<Scene>& scene, const SystemContext& context, float delta_time, const Camera* override_camera = nullptr);

    private:
        MainOpaquePass opaque_pass_{};

        Unique<UniformBuffer> frame_ubo_{nullptr};
        Unique<DescriptorSet> frame_descriptor_set_{nullptr};

        Vector<RenderObject> opaque_queue_{};

        float total_time_{0.0f};
    };
}
