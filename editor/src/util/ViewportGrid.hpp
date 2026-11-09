//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

#include "core/SystemContext.hpp"
#include "render/camera/Camera.hpp"
#include "render/rhi/Buffer.hpp"
#include "render/rhi/DescriptorSet.hpp"
#include "render/rhi/Framebuffer.hpp"
#include "render/rhi/PipelineState.hpp"
#include "render/rhi/Shader.hpp"

namespace editor {
    using namespace engine;

    struct alignas(16) GridUniformData {
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
        glm::mat4 inv_view{1.0f};
        glm::mat4 inv_projection{1.0f};
        glm::vec4 camera_position{0.0f};
        float near_clip{0.1f};
        float far_clip{1000.0f};
        float grid_scale{1.0f};
        float fade_distance{150.0f};
    };
    static_assert(sizeof(GridUniformData) % 16 == 0, "GridUniformData must be 16-byte aligned");

    class ViewportGrid {
    public:
        void init(SystemContext& context);
        void render(SystemContext& context, 
                    const Camera& camera, 
                    Framebuffer* target_framebuffer = nullptr);

        void set_scale(float scale) { scale_ = scale; }
        void set_fade_distance(float distance) { fade_distance_ = distance; }

    private:
        Shared<Shader> grid_shader_{nullptr};
        Unique<PipelineState> pipeline_{nullptr};
        Unique<UniformBuffer> grid_ubo_{nullptr};
        Unique<DescriptorSet> descriptor_set_{nullptr};

        float scale_{1.0f};
        float fade_distance_{150.0f};
        bool initialized_{false};
    };
}