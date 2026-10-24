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

        TextureSpecification default_texture_spec{};
        default_texture_spec.width = 1;
        default_texture_spec.height = 1;
        default_texture_spec.format = TextureFormat::RGBA8;
        default_texture_spec.generate_mipmaps = false;

        constexpr uint32_t white_pixel = 0xFFFFFFFF;
        default_white_texture_ = device.create_texture_2d(default_texture_spec, &white_pixel);

        // Neutral tangent-space normal (0, 0, 1) -> R=128 (0x80), G=128 (0x80), B=255 (0xFF), A=255 (0xFF)
        constexpr uint32_t normal_pixel = 0xFFFF8080;
        default_normal_texture_ = device.create_texture_2d(default_texture_spec, &normal_pixel);

        // Neutral multiplier (1.0, 1.0, 1.0, 1.0) so scalar metallic/roughness factors control the value directly
        constexpr uint32_t mr_pixel = 0xFFFFFFFF;
        default_metallic_roughness_texture_ = device.create_texture_2d(default_texture_spec, &mr_pixel);

        pass_descriptor_set_ = device.create_descriptor_set();
        pass_descriptor_set_->write_storage_buffer(1, *object_ssbo_);
        pass_descriptor_set_->write_texture(2, *default_white_texture_);
        pass_descriptor_set_->write_texture(3, *default_normal_texture_);
        pass_descriptor_set_->write_texture(4, *default_metallic_roughness_texture_);
    }

    void MainOpaquePass::shutdown() {
        pass_descriptor_set_.reset();
        object_ssbo_.reset();
        default_white_texture_.reset();
        default_normal_texture_.reset();
        default_metallic_roughness_texture_.reset();
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
            const auto& obj = objects[i];
            instances.push_back(GPUInstanceData{
                .model_matrix    = obj.transform,
                .color           = obj.color,
                .material_params = {obj.metallic, obj.roughness, obj.ao, obj.normal_scale}
            });
        }
        object_ssbo_->set_data(instances.data(), instances.size() * sizeof(GPUInstanceData));

        cb.bind_pipeline(*pipeline_);
        cb.bind_descriptor_set(frame_descriptor_set);

        const Texture2D* currently_bound_albedo = nullptr;
        const Texture2D* currently_bound_normal = nullptr;
        const Texture2D* currently_bound_metallic_roughness = nullptr;
        const Mesh* currently_bound_mesh = nullptr;

        auto bind_texture_if_needed = [&](const Texture2D* target_albedo,
                                          const Texture2D* target_normal,
                                          const Texture2D* target_metallic_roughness) {
            const Texture2D* albedo = target_albedo ? target_albedo : default_white_texture_.get();
            const Texture2D* normal = target_normal ? target_normal : default_normal_texture_.get();
            const Texture2D* mr = target_metallic_roughness ? target_metallic_roughness : default_metallic_roughness_texture_.get();

            bool changed = false;
            if (albedo != currently_bound_albedo) {
                pass_descriptor_set_->write_texture(2, *albedo);
                currently_bound_albedo = albedo;
                changed = true;
            }
            if (normal != currently_bound_normal) {
                pass_descriptor_set_->write_texture(3, *normal);
                currently_bound_normal = normal;
                changed = true;
            }
            if (mr != currently_bound_metallic_roughness) {
                pass_descriptor_set_->write_texture(4, *mr);
                currently_bound_metallic_roughness = mr;
                changed = true;
            }
            if (changed) {
                cb.bind_descriptor_set(*pass_descriptor_set_);
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
                    const Texture2D* albedo_texture = obj.albedo_texture;
                    const Texture2D* normal_texture = obj.normal_texture;
                    const Texture2D* metallic_roughness_texture = obj.metallic_roughness_texture;

                    if (submesh.material_index < obj.mesh->materials().size()) {
                        const auto& material = obj.mesh->materials()[submesh.material_index];
                        if (!albedo_texture) {
                            albedo_texture = material.albedo_map.get();
                        }
                        if (!normal_texture) {
                            normal_texture = material.normal_map.get();
                        }
                        if (!metallic_roughness_texture) {
                            metallic_roughness_texture = material.metallic_roughness_map.get();
                        }
                    }

                    bind_texture_if_needed(albedo_texture,
                                           normal_texture,
                                           metallic_roughness_texture);

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
                bind_texture_if_needed(obj.albedo_texture,
                                       obj.normal_texture,
                                       obj.metallic_roughness_texture);

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
