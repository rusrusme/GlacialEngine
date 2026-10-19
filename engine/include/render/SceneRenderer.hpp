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

        void begin_scene(const Camera& camera);
        void begin_scene(const glm::mat4& view_projection);

        void end_scene(CommandBuffer& cb, Framebuffer& target_framebuffer);

        void submit(const RenderObject& object);
        void submit(const Mesh* mesh, const glm::mat4& transform, const Texture2D* albedo_texture = nullptr);

        void render_scene(Scene& scene, const SystemContext& context, const Camera* override_camera = nullptr);

    private:
        MainOpaquePass opaque_pass_{};

        Unique<UniformBuffer> frame_ubo_{nullptr};
        Unique<DescriptorSet> frame_descriptor_set_{nullptr};

        Vector<RenderObject> opaque_queue_{};
    };
}
