//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "render/mesh/MeshLoader.hpp"

#include <algorithm>

#include "stb_image.h"
#include "core/Log.hpp"

// TODO Add PBR, Normal support
namespace engine {
    static void generate_tangents(Vector<Vertex>& vertices, const Vector<uint32_t>& indices) {
        for (size_t k = 0; k + 2 < indices.size(); k += 3) {
            Vertex& v0 = vertices[indices[k]];
            Vertex& v1 = vertices[indices[k + 1]];
            Vertex& v2 = vertices[indices[k + 2]];

            const glm::vec3 edge1 = v1.position - v0.position;
            const glm::vec3 edge2 = v2.position - v0.position;
            const glm::vec2 delta_uv1 = v1.uv - v0.uv;
            const glm::vec2 delta_uv2 = v2.uv - v0.uv;

            const float denom = delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y;
            const float f = std::abs(denom) < 0.0001f ? 1.0f : 1.0f / denom;

            const glm::vec4 tangent{
                f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x),
                f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y),
                f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z),
                1.0f
            };

            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
        }

        for (auto& v : vertices) {
            const auto t = glm::vec3(v.tangent);
            if (glm::length(t) > 0.0001f) {
                v.tangent = glm::vec4(glm::normalize(t), 1.0f);
            }
            else {
                v.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            }
        }
    }

    static Shared<Texture2D> load_gltf_image(const RHIDevice& device,
                                             const cgltf_image& img,
                                             const std::filesystem::path& gltf_filepath) {
        int width = 0, height = 0, channels = 0;
        stbi_uc* pixels = nullptr;

        stbi_set_flip_vertically_on_load(0);

        if (img.buffer_view) {
            const auto* base_data = static_cast<const uint8_t*>(img.buffer_view->buffer->data);
            const uint8_t* raw_bytes = base_data + img.buffer_view->offset;
            const auto raw_size = static_cast<int>(img.buffer_view->size);

            pixels = stbi_load_from_memory(raw_bytes, raw_size, &width, &height, &channels, 4);
        }
        else if (img.uri) {
            std::filesystem::path img_path = gltf_filepath.parent_path() / img.uri;
            pixels = stbi_load(img_path.string().c_str(), &width, &height, &channels, 4);
        }

        if (!pixels) {
            CORE_ERROR("[MeshLoader] Failed to decode image: '{0}'", img.name ? img.name : "unnamed");
            return nullptr;
        }

        TextureSpecification spec{};
        spec.width = static_cast<uint32_t>(width);
        spec.height = static_cast<uint32_t>(height);
        spec.format = TextureFormat::RGBA8;
        spec.generate_mipmaps = true;

        auto texture = device.create_texture_2d(spec, pixels);
        stbi_image_free(pixels);

        return texture;
    }

    Shared<Mesh> MeshLoader::load(const RHIDevice& device, const std::filesystem::path& filepath) {
        const std::string ext = filepath.extension().string();

        if (ext == ".glb" || ext == ".gltf") {
            return load_gltf(device, filepath);
        }

        CORE_ERROR("[MeshLoader] Unsupported format for file: {}", filepath.string());
        return nullptr;
    }

    Shared<Mesh> MeshLoader::load_gltf(const RHIDevice& device, const std::filesystem::path& filepath) {
        cgltf_options options{};
        cgltf_data* data = nullptr;
        const cgltf_result parse_res = cgltf_parse_file(&options, filepath.string().c_str(), &data);

        if (parse_res != cgltf_result_success) {
            CORE_ERROR("[MeshLoader] Failed to parse glTF: {0}", filepath.string());
            return nullptr;
        }

        if (cgltf_load_buffers(&options, data, filepath.string().c_str()) != cgltf_result_success) {
            CORE_ERROR("[MeshLoader] Failed to load glTF buffers: {0}", filepath.string());
            cgltf_free(data);
            return nullptr;
        }

        Vector<Shared<Texture2D>> loaded_images(data->images_count);
        for (cgltf_size i = 0; i < data->images_count; ++i) {
            loaded_images[i] = load_gltf_image(device, data->images[i], filepath);
        }

        // Helper
        auto resolve_texture = [&](const cgltf_texture_view& view) -> Shared<Texture2D> {
            if (view.texture && view.texture->image) {
                const auto img_idx = static_cast<size_t>(view.texture->image - data->images);
                if (img_idx < loaded_images.size()) {
                    return loaded_images[img_idx];
                }
            }
            return nullptr;
        };

        Vector<Material> materials(data->materials_count);
        for (cgltf_size i = 0; i < data->materials_count; ++i) {
            const auto& gltf_mat = data->materials[i];
            Material& mat = materials[i];

            mat.name = gltf_mat.name ? gltf_mat.name : "Material_" + std::to_string(i);
            mat.double_sided = gltf_mat.double_sided;

            // PBR Metallic-Roughness + Albedo
            if (gltf_mat.has_pbr_metallic_roughness) {
                const auto& pbr = gltf_mat.pbr_metallic_roughness;

                mat.albedo_map = resolve_texture(pbr.base_color_texture);
                mat.metallic_roughness_map = resolve_texture(pbr.metallic_roughness_texture);

                mat.base_color_factor = glm::vec4(
                    pbr.base_color_factor[0],
                    pbr.base_color_factor[1],
                    pbr.base_color_factor[2],
                    pbr.base_color_factor[3]
                );
                mat.metallic_factor = pbr.metallic_factor;
                mat.roughness_factor = pbr.roughness_factor;
            }

            // Normal Map
            if (gltf_mat.normal_texture.texture) {
                mat.normal_map = resolve_texture(gltf_mat.normal_texture);
            }
        }

        Vector<Vertex> all_vertices;
        Vector<uint32_t> all_indices;
        Vector<Submesh> submeshes;

        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            const cgltf_mesh& mesh = data->meshes[i];
            for (cgltf_size j = 0; j < mesh.primitives_count; ++j) {
                const cgltf_primitive& prim = mesh.primitives[j];
                if (prim.attributes_count == 0 || prim.attributes[0].data == nullptr) {
                    continue;
                }

                const cgltf_size vertex_count = prim.attributes[0].data->count;
                Vector<Vertex> prim_vertices(vertex_count);
                bool has_tangents = false;

                for (cgltf_size a = 0; a < prim.attributes_count; ++a) {
                    const cgltf_attribute& attr = prim.attributes[a];
                    if (attr.type == cgltf_attribute_type_position) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &prim_vertices[v].position.x, 3);
                        }
                    }
                    else if (attr.type == cgltf_attribute_type_normal) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &prim_vertices[v].normal.x, 3);
                        }
                    }
                    else if (attr.type == cgltf_attribute_type_texcoord && attr.index == 0) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &prim_vertices[v].uv.x, 2);
                        }
                    }
                    else if (attr.type == cgltf_attribute_type_tangent) {
                        has_tangents = true;
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &prim_vertices[v].tangent.x, 4);
                        }
                    }
                }

                Vector<uint32_t> prim_indices;
                if (prim.indices) {
                    prim_indices.resize(prim.indices->count);
                    for (cgltf_size k = 0; k < prim.indices->count; ++k) {
                        prim_indices[k] = static_cast<uint32_t>(cgltf_accessor_read_index(prim.indices, k));
                    }
                }
                else {
                    prim_indices.resize(vertex_count);
                    for (uint32_t k = 0; k < vertex_count; ++k) prim_indices[k] = k;
                }

                if (!has_tangents && !prim_indices.empty()) {
                    generate_tangents(prim_vertices, prim_indices);
                }

                AABB submesh_bounds{};
                if (!prim_vertices.empty()) {
                    submesh_bounds.min = prim_vertices[0].position;
                    submesh_bounds.max = prim_vertices[0].position;
                    for (const auto& v : prim_vertices) {
                        submesh_bounds.min = glm::min(submesh_bounds.min, v.position);
                        submesh_bounds.max = glm::max(submesh_bounds.max, v.position);
                    }
                }

                uint32_t mat_index = 0;
                if (prim.material) {
                    mat_index = static_cast<uint32_t>(prim.material - data->materials);
                }

                Submesh submesh{};
                submesh.first_index = static_cast<uint32_t>(all_indices.size());
                submesh.vertex_offset = static_cast<int32_t>(all_vertices.size());
                submesh.index_count = static_cast<uint32_t>(prim_indices.size());
                submesh.material_index = mat_index;
                submesh.bounds = submesh_bounds;
                submeshes.push_back(submesh);

                all_vertices.insert(all_vertices.end(), prim_vertices.begin(), prim_vertices.end());
                all_indices.insert(all_indices.end(), prim_indices.begin(), prim_indices.end());
            }
        }

        cgltf_free(data);

        if (all_vertices.empty() || all_indices.empty()) {
            CORE_ERROR("[MeshLoader] Loaded empty glTF mesh from: {0}", filepath.string());
            return nullptr;
        }

        return std::make_shared<Mesh>(device, all_vertices, all_indices, std::move(submeshes), std::move(materials));
    }
}
