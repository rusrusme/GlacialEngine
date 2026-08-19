//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/mesh/MeshLoader.hpp"

#include <fstream>
#include <iostream>
#include <render/mesh/Mesh.hpp>
#include <render/mesh/PrimitiveMesh.hpp>
#include <spdlog/sinks/basic_file_sink-inl.h>

namespace engine {
    Mesh MeshLoader::load(const std::string& path) {
        if (path == "primitive://cube") return PrimitiveMesh::create_cube();
        if (path == "primitive://plane") return PrimitiveMesh::create_plane();
        if (path == "primitive://sphere") return PrimitiveMesh::create_uv_sphere();
        if (path == "primitive://circle") return PrimitiveMesh::create_circle();
        if (path == "primitive://cylinder") return PrimitiveMesh::create_cylinder();

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
