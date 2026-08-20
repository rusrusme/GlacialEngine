//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "render/mesh/MeshLoader.hpp"

#include <fstream>
#include <iostream>
#include <render/mesh/Mesh.hpp>
#include <render/mesh/PrimitiveMesh.hpp>
#include <spdlog/sinks/basic_file_sink-inl.h>

namespace engine {
    static Mesh load_gltf(const std::string& path) {
        cgltf_options options{};
        cgltf_data* data = nullptr;
        cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);

        if (result != cgltf_result_success) {
            std::cerr << "Engine error: Failed to parse glTF/GLB: " << path << "\n";
            return {};
        }

        if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
            cgltf_free(data);
            return {};
        }

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            const auto& mesh = data->meshes[i];
            for (cgltf_size j = 0; j < mesh.primitives_count; ++j) {
                const auto& prim = mesh.primitives[j];

                cgltf_size vertex_count = prim.attributes[0].data->count;
                cgltf_size base_index = vertices.size();
                vertices.resize(base_index + vertex_count);

                for (cgltf_size a = 0; a < prim.attributes_count; ++a) {
                    const auto& attr = prim.attributes[a];
                    if (attr.type == cgltf_attribute_type_position) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &vertices[base_index + v].position.x, 3);
                        }
                    }
                    else if (attr.type == cgltf_attribute_type_normal) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &vertices[base_index + v].normal.x, 3);
                        }
                    }
                    else if (attr.type == cgltf_attribute_type_texcoord) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &vertices[base_index + v].uv.x, 2);
                        }
                    }
                }
                if (prim.indices) {
                    cgltf_size index_count = prim.indices->count;
                    cgltf_size idx_base = indices.size();
                    indices.resize(idx_base + index_count);
                    for (cgltf_size k = 0; k < index_count; ++k) {
                        indices[idx_base + k] = static_cast<uint32_t>(base_index + cgltf_accessor_read_index(prim.indices, k));
                    }
                }
            }
        }
        cgltf_free(data);
        return {vertices, indices};
    }

    Mesh MeshLoader::load(const std::string& path) {
        if (path == "primitive://cube") return PrimitiveMesh::create_cube();
        if (path == "primitive://plane") return PrimitiveMesh::create_plane();
        if (path == "primitive://sphere") return PrimitiveMesh::create_uv_sphere();
        if (path == "primitive://circle") return PrimitiveMesh::create_circle();
        if (path == "primitive://cylinder") return PrimitiveMesh::create_cylinder();

        if (path.ends_with(".glb") || path.ends_with(".gltf")) return load_gltf(path);

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Engine error: mesh loader: failed to open file: " << path << std::endl;
            return Mesh{};
        }

        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_normals;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

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

        return {vertices, indices};
    }
}
