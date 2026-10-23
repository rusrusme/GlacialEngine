//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/pass/MainOpaquePass.hpp"

#include "render/ShaderData.hpp"
#include "render/mesh/Mesh.hpp"

// TODO if material has double_sided -> pipeline state no cull!
namespace engine {
    void MainOpaquePass::init(const RHIDevice& device) {
        shader_ = std::make_shared<Shader>("assets/shaders/compiled/OpaqueMesh.gsb");

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

        // SSBO for Per-Object Transforms (Slot 1)
        object_ssbo_ = device.create_shader_storage_buffer(max_objects_ * sizeof(GPUInstanceData), 1);

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

    void MainOpaquePass::shutdown() {
        pass_descriptor_set_.reset();
        object_ssbo_.reset();
        default_white_texture_.reset();
        pipeline_.reset();
        shader_.reset();
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

        Vector<GPUInstanceData> instances;
        instances.reserve(draw_count);
        for (size_t i = 0; i < draw_count; ++i) {
            instances.push_back(GPUInstanceData{
                .model_matrix = objects[i].transform,
                .color        = objects[i].color
            });
        }
        object_ssbo_->set_data(instances.data(), instances.size() * sizeof(GPUInstanceData));

        cb.bind_pipeline(*pipeline_);
        cb.bind_descriptor_set(frame_descriptor_set);

        const Texture2D* currently_bound_texture = nullptr;
        const Mesh* currently_bound_mesh = nullptr;

        auto bind_texture_if_needed = [&](const Texture2D* target_tex) {
            const Texture2D* tex = target_tex ? target_tex : default_white_texture_.get();
            if (tex != currently_bound_texture) {
                pass_descriptor_set_->write_texture(2, *tex);
                cb.bind_descriptor_set(*pass_descriptor_set_);
                currently_bound_texture = tex;
            }
        };

        for (uint32_t i = 0; i < static_cast<uint32_t>(draw_count); ++i) {
            const auto& obj = objects[i];
            if (!obj.mesh) continue;

            if (obj.mesh != currently_bound_mesh) {
                cb.bind_vertex_buffer(0, obj.mesh->vertex_buffer(), 0, sizeof(Vertex));
                cb.bind_index_buffer(obj.mesh->index_buffer(), IndexType::UInt32);
                currently_bound_mesh = obj.mesh;
            }

            if (!obj.mesh->submeshes().empty()) {
                for (const auto& submesh : obj.mesh->submeshes()) {
                    const Texture2D* tex = obj.albedo_texture;

                    if (!tex && submesh.material_index < obj.mesh->materials().size()) {
                        tex = obj.mesh->materials()[submesh.material_index].albedo_map.get();
                    }

                    bind_texture_if_needed(tex);

                    cb.draw_indexed(
                        submesh.index_count,
                        1,
                        submesh.first_index,
                        submesh.vertex_offset,
                        i // first_instance -> SSBO Transform-Index
                    );
                }
            }
            else {
                bind_texture_if_needed(obj.albedo_texture);

                cb.draw_indexed(
                    obj.mesh->index_count(),
                    1,
                    0,
                    0,
                    i
                );
            }
        }

        cb.end_render_pass();
    }
}
