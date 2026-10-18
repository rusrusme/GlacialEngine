//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/pass/MainOpaquePass.hpp"

#include "render/mesh/Mesh.hpp"

namespace engine {
    void MainOpaquePass::init(const RHIDevice& device) {
        shader_ = std::make_shared<Shader>("assets/shaders/compiled/OpaqueMesh.bshb");

        PipelineStateDesc desc{};
        desc.shader = shader_;
        desc.vertex_layout = Vertex::layout();
        desc.cull_mode = CullMode::Back;
        desc.polygon_mode = PolygonMode::Fill;
        desc.depth_test = true;
        desc.depth_write = true;
        desc.depth_compare = CompareOp::Less;
        desc.blend_enable = false;
        desc.blend_mode = BlendMode::Opaque;

        pipeline_ = device.create_pipeline_state(desc);

        object_ssbo_ = device.create_shader_storage_buffer(max_objects_ * sizeof(glm::mat4), 1);

        pass_descriptor_set_ = device.create_descriptor_set();
        pass_descriptor_set_->write_storage_buffer(1, *object_ssbo_);
    }

    void MainOpaquePass::execute(CommandBuffer& cb,
                                 Framebuffer& target_framebuffer,
                                 const DescriptorSet& frame_descriptor_set,
                                 std::span<const RenderObject> objects) {
        if (objects.empty()) {
            return;
        }

        std::vector<glm::mat4> transforms;
        transforms.reserve(objects.size());
        for (const auto& [mesh, transform] : objects) {
            transforms.push_back(transform);
        }
        object_ssbo_->set_data(transforms.data(), transforms.size() * sizeof(glm::mat4));

        constexpr glm::vec4 clear_color = {0.1f, 0.1f, 0.12f, 1.0f};
        cb.begin_render_pass(target_framebuffer, clear_color);

        cb.bind_pipeline(*pipeline_);
        cb.bind_descriptor_set(frame_descriptor_set);  // Slot 0: Kamera (ViewProjection)
        cb.bind_descriptor_set(*pass_descriptor_set_); // Slot 1: Objekt-Transforms

        for (uint32_t i = 0; i < objects.size(); ++i) {
            const auto* mesh = objects[i].mesh;
            if (!mesh) continue;

            cb.bind_vertex_buffer(0, mesh->vertex_buffer(), 0, sizeof(Vertex));
            cb.bind_index_buffer(mesh->index_buffer(), IndexType::UInt32);

            for (const auto& submesh : mesh->submeshes()) {
                // first_instance = i übergibt den Index für u_Models[gl_BaseInstance]
                cb.draw_indexed(
                    submesh.index_count,
                    1,
                    submesh.first_index,
                    submesh.vertex_offset,
                    i // first_instance
                );
            }
        }

        cb.end_render_pass();
    }
}
