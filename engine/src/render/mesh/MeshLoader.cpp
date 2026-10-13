//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "render/mesh/MeshLoader.hpp"

#include "render/gpu/GPUMeshManager.hpp"
#include "render/gpu/MaterialSystem.hpp"
#include "render/gpu/TextureSystem.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>

#include "core/Log.hpp"

namespace engine {
    static void generate_tangents(Vector<Vertex>& vertices, const Vector<uint32_t>& indices) {
        for (size_t k = 0; k < indices.size(); k += 3) {
            Vertex& v0 = vertices[indices[k]];
            Vertex& v1 = vertices[indices[k + 1]];
            Vertex& v2 = vertices[indices[k + 2]];

            const glm::vec3 edge1 = v1.position - v0.position;
            const glm::vec3 edge2 = v2.position - v0.position;
            const glm::vec2 deltaUV1 = v1.uv - v0.uv;
            const glm::vec2 deltaUV2 = v2.uv - v0.uv;

            const float denom = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
            const float f = std::abs(denom) < 0.0001f ? 1.0f : 1.0f / denom;

            const glm::vec4 tangent{
                f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
                1.0f
            };

            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
        }

        for (auto& v : vertices) {
            auto t = glm::vec3(v.tangent);
            if (glm::length(t) > 0.0001f) {
                v.tangent = glm::vec4(glm::normalize(t), 1.0f);
            }
            else {
                v.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            }
        }
    }

    static Submesh process_geometry(Vector<Vertex>& vertices,
                                    Vector<uint32_t>& indices,
                                    uint32_t material_id,
                                    GPUMeshManager& mesh_manager) {
        GPUAABB submesh_bounds{
            .min = glm::vec4(std::numeric_limits<float>::max()),
            .max = glm::vec4(-std::numeric_limits<float>::max())
        };

        for (const auto& v : vertices) {
            submesh_bounds.min = glm::min(submesh_bounds.min, glm::vec4(v.position, 1.0f));
            submesh_bounds.max = glm::max(submesh_bounds.max, glm::vec4(v.position, 1.0f));
        }

        const size_t vertex_bytes = vertices.size() * sizeof(Vertex);
        GPUMeshAllocation alloc = mesh_manager.register_submesh(
            vertices.data(),
            vertex_bytes,
            static_cast<uint32_t>(vertices.size()),
            indices.data(),
            static_cast<uint32_t>(indices.size()),
            submesh_bounds
        );

        Submesh submesh{};
        submesh.mesh_id = alloc.mesh_id;
        submesh.first_index = alloc.first_index;
        submesh.base_vertex = alloc.base_vertex;
        submesh.index_count = alloc.index_count;
        submesh.default_material_id = material_id;
        submesh.local_bounds.min = glm::vec3(submesh_bounds.min);
        submesh.local_bounds.max = glm::vec3(submesh_bounds.max);

        return submesh;
    }

    // Public Entry Point
    Mesh MeshLoader::load(const std::string& filepath,
                          GPUMeshManager& mesh_manager,
                          MaterialSystem& material_system,
                          TextureSystem& texture_system) {
        if (filepath.rfind("primitive://", 0) == 0) {
            return load_primitive(filepath, mesh_manager, material_system);
        }

        if (filepath.ends_with(".glb") || filepath.ends_with(".gltf")) {
            return load_gltf(filepath, mesh_manager, material_system, texture_system);
        }

        if (filepath.ends_with(".obj")) {
            return load_obj(filepath, mesh_manager, material_system);
        }

        CORE_ERROR("[MeshLoader] Unsupported format for file: {}", filepath.c_str());
        return Mesh{};
    }

    Mesh MeshLoader::load_primitive(const std::string& path,
                                    GPUMeshManager& mesh_manager,
                                    MaterialSystem& material_system) {
        Mesh mesh_asset;
        mesh_asset.name_ = path;

        Vector<Vertex> vertices;
        Vector<uint32_t> indices;

        if (path == "primitive://cube") {
            vertices = {
                // Front
                {.position = {-0.5f, -0.5f, 0.5f}, .normal = {0, 0, 1}, .uv = {0, 0}, .tangent = {1, 0, 0, 1}},
                {.position = {0.5f, -0.5f, 0.5f}, .normal = {0, 0, 1}, .uv = {1, 0}, .tangent = {1, 0, 0, 1}},
                {.position = {0.5f, 0.5f, 0.5f}, .normal = {0, 0, 1}, .uv = {1, 1}, .tangent = {1, 0, 0, 1}},
                {.position = {-0.5f, 0.5f, 0.5f}, .normal = {0, 0, 1}, .uv = {0, 1}, .tangent = {1, 0, 0, 1}},
                // Back
                {.position = {0.5f, -0.5f, -0.5f}, .normal = {0, 0, -1}, .uv = {0, 0}, .tangent = {-1, 0, 0, 1}},
                {.position = {-0.5f, -0.5f, -0.5f}, .normal = {0, 0, -1}, .uv = {1, 0}, .tangent = {-1, 0, 0, 1}},
                {.position = {-0.5f, 0.5f, -0.5f}, .normal = {0, 0, -1}, .uv = {1, 1}, .tangent = {-1, 0, 0, 1}},
                {.position = {0.5f, 0.5f, -0.5f}, .normal = {0, 0, -1}, .uv = {0, 1}, .tangent = {-1, 0, 0, 1}},
            };
            indices = {
                0, 1, 2, 2, 3, 0,
                4, 5, 6, 6, 7, 4,
                3, 2, 7, 7, 6, 3,
                5, 4, 1, 1, 0, 5,
                5, 0, 3, 3, 6, 5,
                1, 4, 7, 7, 2, 1
            };
        }
        else if (path == "primitive://plane") {
            vertices = {
                {.position = {-0.5f, 0.0f, -0.5f}, .normal = {0, 1, 0}, .uv = {0, 0}, .tangent = {1, 0, 0, 1}},
                {.position = {0.5f, 0.0f, -0.5f}, .normal = {0, 1, 0}, .uv = {1, 0}, .tangent = {1, 0, 0, 1}},
                {.position = {0.5f, 0.0f, 0.5f}, .normal = {0, 1, 0}, .uv = {1, 1}, .tangent = {1, 0, 0, 1}},
                {.position = {-0.5f, 0.0f, 0.5f}, .normal = {0, 1, 0}, .uv = {0, 1}, .tangent = {1, 0, 0, 1}}
            };
            indices = {0, 1, 2, 2, 3, 0};
        }

        GPUMaterialData default_mat{};
        uint32_t mat_id = material_system.register_material(default_mat);

        Submesh submesh = process_geometry(vertices, indices, mat_id, mesh_manager);
        mesh_asset.submeshes_.push_back(submesh);
        mesh_asset.bounds_ = submesh.local_bounds;

        return mesh_asset;
    }

    Mesh MeshLoader::load_gltf(const std::string& path,
                               GPUMeshManager& mesh_manager,
                               MaterialSystem& material_system,
                               TextureSystem& texture_system) {
        Mesh mesh_asset;
        mesh_asset.name_ = std::filesystem::path(path).stem().string();

        cgltf_options options{};
        cgltf_data* data = nullptr;
        cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);

        if (result != cgltf_result_success) {
            CORE_ERROR("[MeshLoader] Failed to parse glTF/GLB: {}", path.c_str());
            return mesh_asset;
        }

        if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
            cgltf_free(data);
            return mesh_asset;
        }

        const std::string directory = std::filesystem::path(path).parent_path().string();

        Vector<uint32_t> material_ids;
        material_ids.reserve(data->materials_count);

        for (cgltf_size i = 0; i < data->materials_count; ++i) {
            const cgltf_material& mat = data->materials[i];
            GPUMaterialData mat_data{};

            if (mat.has_pbr_metallic_roughness) {
                mat_data.albedo_factor = glm::vec4(
                    mat.pbr_metallic_roughness.base_color_factor[0],
                    mat.pbr_metallic_roughness.base_color_factor[1],
                    mat.pbr_metallic_roughness.base_color_factor[2],
                    mat.pbr_metallic_roughness.base_color_factor[3]
                );
                mat_data.metallic_factor = mat.pbr_metallic_roughness.metallic_factor;
                mat_data.roughness_factor = mat.pbr_metallic_roughness.roughness_factor;

                if (mat.pbr_metallic_roughness.base_color_texture.texture) {
                    const cgltf_image* img = mat.pbr_metallic_roughness.base_color_texture.texture->image;
                    if (img && img->uri) {
                        std::string tex_path = (std::filesystem::path(directory) / img->uri).string();
                        mat_data.albedo_texture_index = texture_system.register_texture(tex_path);
                    }
                }

                if (mat.pbr_metallic_roughness.metallic_roughness_texture.texture) {
                    const cgltf_image* img = mat.pbr_metallic_roughness.metallic_roughness_texture.texture->image;
                    if (img && img->uri) {
                        std::string tex_path = (std::filesystem::path(directory) / img->uri).string();
                        mat_data.metallic_roughness_index = texture_system.register_texture(tex_path);
                    }
                }
            }

            if (mat.normal_texture.texture) {
                const cgltf_image* img = mat.normal_texture.texture->image;
                if (img && img->uri) {
                    std::string tex_path = (std::filesystem::path(directory) / img->uri).string();
                    mat_data.normal_texture_index = texture_system.register_texture(tex_path);
                }
            }

            material_ids.push_back(material_system.register_material(mat_data));
        }

        GPUMaterialData default_mat{};
        uint32_t default_mat_id = material_system.register_material(default_mat);

        glm::vec3 global_min(std::numeric_limits<float>::max());
        glm::vec3 global_max(-std::numeric_limits<float>::max());

        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            const cgltf_mesh& mesh = data->meshes[i];
            for (cgltf_size j = 0; j < mesh.primitives_count; ++j) {
                const cgltf_primitive& prim = mesh.primitives[j];

                if (prim.attributes_count == 0 || prim.attributes[0].data == nullptr) {
                    continue;
                }

                cgltf_size vertex_count = prim.attributes[0].data->count;
                Vector<Vertex> vertices(vertex_count);
                bool has_tangents = false;

                for (cgltf_size a = 0; a < prim.attributes_count; ++a) {
                    const cgltf_attribute& attr = prim.attributes[a];
                    if (attr.type == cgltf_attribute_type_position) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &vertices[v].position.x, 3);
                        }
                    }
                    else if (attr.type == cgltf_attribute_type_normal) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &vertices[v].normal.x, 3);
                        }
                    }
                    else if (attr.type == cgltf_attribute_type_texcoord) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &vertices[v].uv.x, 2);
                        }
                    }
                    else if (attr.type == cgltf_attribute_type_tangent) {
                        has_tangents = true;
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &vertices[v].tangent.x, 4);
                        }
                    }
                }

                Vector<uint32_t> indices;
                if (prim.indices) {
                    indices.resize(prim.indices->count);
                    for (cgltf_size k = 0; k < prim.indices->count; ++k) {
                        indices[k] = static_cast<uint32_t>(cgltf_accessor_read_index(prim.indices, k));
                    }
                }

                if (!has_tangents && !indices.empty()) {
                    generate_tangents(vertices, indices);
                }

                uint32_t mat_id = default_mat_id;
                if (prim.material) {
                    auto it = std::find_if(data->materials, data->materials + data->materials_count,
                                           [&](const cgltf_material& m) { return &m == prim.material; });
                    if (it != data->materials + data->materials_count) {
                        mat_id = material_ids[std::distance(data->materials, it)];
                    }
                }

                Submesh submesh = process_geometry(vertices, indices, mat_id, mesh_manager);
                mesh_asset.submeshes_.push_back(submesh);

                global_min = glm::min(global_min, submesh.local_bounds.min);
                global_max = glm::max(global_max, submesh.local_bounds.max);
            }
        }

        cgltf_free(data);

        mesh_asset.bounds_.min = global_min;
        mesh_asset.bounds_.max = global_max;

        return mesh_asset;
    }

    Mesh MeshLoader::load_obj(const std::string& path,
                              GPUMeshManager& mesh_manager,
                              MaterialSystem& material_system) {
        Mesh mesh_asset;
        mesh_asset.name_ = std::filesystem::path(path).stem().string();

        std::ifstream file(path);
        if (!file.is_open()) {
            CORE_ERROR("[MeshLoader] Failed to open OBJ file: {}", path.c_str());
            return mesh_asset;
        }

        Vector<glm::vec3> temp_positions;
        Vector<glm::vec2> temp_uvs;
        Vector<glm::vec3> temp_normals;

        Vector<Vertex> vertices;
        Vector<uint32_t> indices;

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                glm::vec3 pos;
                ss >> pos.x >> pos.y >> pos.z;
                temp_positions.push_back(pos);
            }
            else if (prefix == "vt") {
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                temp_uvs.push_back(uv);
            }
            else if (prefix == "vn") {
                glm::vec3 norm;
                ss >> norm.x >> norm.y >> norm.z;
                temp_normals.push_back(norm);
            }
            else if (prefix == "f") {
                std::string vertex_str;
                while (ss >> vertex_str) {
                    std::istringstream vss(vertex_str);
                    std::string p_idx, t_idx, n_idx;

                    std::getline(vss, p_idx, '/');
                    std::getline(vss, t_idx, '/');
                    std::getline(vss, n_idx, '/');

                    Vertex vertex{};
                    if (!p_idx.empty()) vertex.position = temp_positions[std::stoi(p_idx) - 1];
                    if (!t_idx.empty()) vertex.uv = temp_uvs[std::stoi(t_idx) - 1];
                    if (!n_idx.empty()) vertex.normal = temp_normals[std::stoi(n_idx) - 1];

                    vertices.push_back(vertex);
                    indices.push_back(static_cast<uint32_t>(indices.size()));
                }
            }
        }

        generate_tangents(vertices, indices);

        GPUMaterialData default_mat{};
        uint32_t mat_id = material_system.register_material(default_mat);

        Submesh submesh = process_geometry(vertices, indices, mat_id, mesh_manager);
        mesh_asset.submeshes_.push_back(submesh);
        mesh_asset.bounds_ = submesh.local_bounds;

        return mesh_asset;
    }
}
