//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ViewportGrid.hpp"

#include "core/Log.hpp"
#include "render/rhi/RenderContext.hpp"
#include "render/shader/ShaderLibrary.hpp"

namespace editor {
    void ViewportGrid::init(SystemContext& context) {
        if (initialized_) return;

        auto& device = context.render_context.device();

        grid_shader_ = context.shaders.load(
            "grid",
            "assets/shaders/compiled/editor/Grid.gsb"
        );
        if (!grid_shader_ || grid_shader_->renderer_id() == 0) {
            CORE_ERROR("[ViewportGrid] Failed to load shader: assets/shaders/compiled/editor/Grid.gsb");
            return;
        }

        PipelineStateDesc desc{};
        desc.shader = grid_shader_;
        desc.vertex_layout = {};
        desc.cull_mode = CullMode::None;
        desc.polygon_mode = PolygonMode::Fill;
        desc.depth_test = true;
        desc.depth_write = false; // Grid is translucent; do not overwrite depth buffer
        desc.depth_compare = CompareOp::LessOrEqual;
        desc.blend_enable = true;
        desc.blend_mode = BlendMode::Alpha;

        pipeline_ = device.create_pipeline_state(desc);

        grid_ubo_ = device.create_uniform_buffer(sizeof(GridUniformData), 0);
        descriptor_set_ = device.create_descriptor_set();
        descriptor_set_->write_uniform_buffer(0, *grid_ubo_);

        initialized_ = true;
    }

    void ViewportGrid::render(SystemContext& context, const Camera& camera, Framebuffer* target_framebuffer) {
        if (!initialized_ || !grid_shader_ || !pipeline_) return;

        GridUniformData ubo_data;
        ubo_data.view = camera.view();
        ubo_data.projection = camera.projection();
        ubo_data.inv_view = glm::inverse(camera.view());
        ubo_data.inv_projection = glm::inverse(camera.projection());
        ubo_data.camera_position = glm::vec4(camera.position(), camera.distance());
        ubo_data.near_clip = 0.1f;
        ubo_data.far_clip = 1000.0f;
        ubo_data.grid_scale = scale_;
        ubo_data.fade_distance = fade_distance_;

        grid_ubo_->set_data(&ubo_data, sizeof(GridUniformData));

        auto& cb = context.render_context.active_command_buffer();
        Framebuffer& target_fb = target_framebuffer
                                     ? *target_framebuffer
                                     : context.render_context.swapchain().backbuffer();

        RenderPassBeginInfo begin_info{};
        begin_info.framebuffer = &target_fb;
        begin_info.clear_color_target = false;
        begin_info.clear_depth_target = false;
        begin_info.clear_stencil_target = false;

        cb.begin_render_pass(begin_info);

        cb.bind_pipeline(*pipeline_);
        cb.bind_descriptor_set(*descriptor_set_);
        cb.draw(6);

        cb.end_render_pass();
    }
}