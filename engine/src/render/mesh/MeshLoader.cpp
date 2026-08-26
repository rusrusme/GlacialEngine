//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <stb_image.h>

#include "render/mesh/MeshLoader.hpp"

#include "render/mesh/Mesh.hpp"
#include "render/mesh/PrimitiveMesh.hpp"

namespace engine {
    static std::shared_ptr<Texture2D> extract_gltf_texture(const cgltf_texture* gltf_tex) {
        if (!gltf_tex || !gltf_tex->image) return nullptr;

        const cgltf_image* image = gltf_tex->image;
        const uint8_t* buffer_data = nullptr;
        size_t buffer_size = 0;

        if (image->buffer_view) {
            const auto* buffer = image->buffer_view->buffer;
            buffer_data = static_cast<const uint8_t*>(buffer->data) + image->buffer_view->offset;
            buffer_size = image->buffer_view->size;
        }

        if (!buffer_data || buffer_size == 0) return nullptr;

        int width, height, channels;
        stbi_uc* pixels = stbi_load_from_memory(buffer_data, static_cast<int>(buffer_size), &width, &height, &channels, 4);
        if (!pixels) {
            std::cerr << "Engine warning: Failed to decode embedded GLB texture!\n";
            return nullptr;
        }

        auto texture = std::make_shared<Texture2D>(width, height, 4, pixels);
        stbi_image_free(pixels);
        return texture;
    }

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
        std::shared_ptr<Texture2D> albedo_texture = nullptr;
        std::shared_ptr<Texture2D> normal_texture = nullptr;

        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            const auto& mesh = data->meshes[i];
            for (cgltf_size j = 0; j < mesh.primitives_count; ++j) {
                const auto& prim = mesh.primitives[j];

                if (prim.material) {
                    if (!albedo_texture && prim.material->has_pbr_metallic_roughness) {
                        albedo_texture = extract_gltf_texture(prim.material->pbr_metallic_roughness.base_color_texture.texture);
                    }
                    if (!normal_texture && prim.material->normal_texture.texture) {
                        normal_texture = extract_gltf_texture(prim.material->normal_texture.texture); // <--- Extract normal map
                    }
                }

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
                    else if (attr.type == cgltf_attribute_type_tangent) {
                        for (cgltf_size v = 0; v < vertex_count; ++v) {
                            cgltf_accessor_read_float(attr.data, v, &vertices[base_index + v].tangent.x, 3);
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

        if (vertices.empty() || indices.empty()) return {};

        for (size_t k = 0; k < indices.size(); k += 3) {
            Vertex& v0 = vertices[indices[k]];
            Vertex& v1 = vertices[indices[k + 1]];
            Vertex& v2 = vertices[indices[k + 2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            glm::vec2 deltaUV1 = v1.uv - v0.uv;
            glm::vec2 deltaUV2 = v2.uv - v0.uv;

            float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            float f = (std::abs(denom) < 0.0001f) ? 1.0f : 1.0f / denom;

            glm::vec3 tangent{
                f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
            };

            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
        }

        for (auto& v : vertices) {
            if (glm::length(v.tangent) > 0.0001f) {
                v.tangent = glm::normalize(v.tangent);
            } else {
                v.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }

        AABB local_bounds{
            glm::vec3(std::numeric_limits<float>::max()),
            glm::vec3(std::numeric_limits<float>::lowest())
        };

        for (const auto& v : vertices) {
            local_bounds.min = glm::min(local_bounds.min, v.position);
            local_bounds.max = glm::max(local_bounds.max, v.position);
        }

        auto vao = std::make_shared<VertexArray>();
        const auto vbo = std::make_shared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
        const BufferLayout layout = {
            {ShaderDataType::Float3, "a_position"},
            {ShaderDataType::Float3, "a_normal"},
            {ShaderDataType::Float2, "a_uv"},
            {ShaderDataType::Float3, "a_tangent"}
        };
        vao->add_vertex_buffer(vbo, layout);

        const auto ebo = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
        vao->set_index_buffer(ebo);

        Mesh mesh{};
        mesh.vertex_array = std::move(vao);
        mesh.texture = std::move(albedo_texture);
        mesh.normal_map = std::move(normal_texture);
        mesh.local_bounds = local_bounds;
        return mesh;
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

        for (size_t k = 0; k < indices.size(); k += 3) {
            Vertex& v0 = vertices[indices[k]];
            Vertex& v1 = vertices[indices[k + 1]];
            Vertex& v2 = vertices[indices[k + 2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            glm::vec2 deltaUV1 = v1.uv - v0.uv;
            glm::vec2 deltaUV2 = v2.uv - v0.uv;

            float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            float f = (std::abs(denom) < 0.0001f) ? 1.0f : 1.0f / denom;

            glm::vec3 tangent{
                f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
            };

            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
        }

        for (auto& v : vertices) {
            if (glm::length(v.tangent) > 0.0001f) v.tangent = glm::normalize(v.tangent);
            else v.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        return {vertices, indices};
    }
}
