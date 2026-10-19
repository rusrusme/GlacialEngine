//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/pass/MainOpaquePass.hpp"

#include "render/graph/RenderGraph.hpp"
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

        TextureSpecification white_spec{};
        white_spec.width = 1;
        white_spec.height = 1;
        white_spec.format = TextureFormat::RGBA8;
        white_spec.generate_mipmaps = false;

        constexpr uint32_t white_pixel = 0xFFFFFFFF;
        default_white_texture_ = device.create_texture_2d(white_spec, &white_pixel);

        pass_descriptor_set_ = device.create_descriptor_set();
        pass_descriptor_set_->write_storage_buffer(1, *object_ssbo_);
        pass_descriptor_set_->write_texture(2, *default_white_texture_);
    }

    void MainOpaquePass::execute(CommandBuffer& cb,
                                 Framebuffer& target_framebuffer,
                                 const DescriptorSet& frame_descriptor_set,
                                 std::span<const RenderObject> objects) {
        constexpr glm::vec4 clear_color = {0.1f, 0.1f, 0.12f, 1.0f};
        cb.begin_render_pass(target_framebuffer, clear_color);

        if (objects.empty()) {
            cb.end_render_pass();
            return;
        }

        const size_t draw_count = std::min(objects.size(), max_objects_);

        std::vector<glm::mat4> transforms;
        transforms.reserve(draw_count);
        for (size_t i = 0; i < draw_count; ++i) {
            transforms.push_back(objects[i].transform);
        }
        object_ssbo_->set_data(transforms.data(), transforms.size() * sizeof(glm::mat4));

        cb.bind_pipeline(*pipeline_);
        cb.bind_descriptor_set(frame_descriptor_set); // Slot 0: FrameData UBO

        for (uint32_t i = 0; i < static_cast<uint32_t>(draw_count); ++i) {
            const auto& obj = objects[i];
            if (!obj.mesh) continue;

            const auto* texture = obj.albedo_texture ? obj.albedo_texture : default_white_texture_.get();
            pass_descriptor_set_->write_texture(2, *texture);
            cb.bind_descriptor_set(*pass_descriptor_set_);

            cb.bind_vertex_buffer(0, obj.mesh->vertex_buffer(), 0, sizeof(Vertex));
            cb.bind_index_buffer(obj.mesh->index_buffer(), IndexType::UInt32);

            for (const auto& submesh : obj.mesh->submeshes()) {
                cb.draw_indexed(
                    submesh.index_count,
                    1,
                    submesh.first_index,
                    submesh.vertex_offset,
                    i // first_instance -> a_ObjectIndex
                );
            }
        }
        cb.end_render_pass();
    }

    // Exemplary setup
    // void MainOpaquePass::add_to_graph(RenderGraph& graph,
    //                                   std::span<const RenderObject> objects,
    //                                   const std::string& out_color,
    //                                   const std::string& out_depth) {
    //     struct OpaquePassData {
    //         std::span<const RenderObject> objects;
    //     };
    //     graph.add_pass<OpaquePassData>(
    //         "MainOpaque",
    //         [&](RenderGraphBuilder& builder, OpaquePassData& data) {
    //             data.objects = objects;
    //             builder.write_texture(out_color);
    //             builder.write_texture(out_depth);
    //         },
    //         [this](const OpaquePassData& data, const RenderGraphRegistry& reg, CommandBuffer& cb) {
    //             cb.bind_pipeline(*pipeline_);
    //             for (const auto& obj : data.objects) {
    //                 // ...
    //             }
    //         });
    // }
}
