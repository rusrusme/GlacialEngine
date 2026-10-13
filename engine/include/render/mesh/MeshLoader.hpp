//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

#include "render/mesh/Mesh.hpp"

namespace engine {
    class GPUMeshManager;
    class MaterialSystem;
    class TextureSystem;

    class MeshLoader {
    public:
        static Mesh load(const std::string& filepath,
                         GPUMeshManager& mesh_manager,
                         MaterialSystem& material_system,
                         TextureSystem& texture_system);

    private:
        static Mesh load_primitive(const std::string& path,
                                   GPUMeshManager& mesh_manager,
                                   MaterialSystem& material_system);

        static Mesh load_gltf(const std::string& path,
                              GPUMeshManager& mesh_manager,
                              MaterialSystem& material_system,
                              TextureSystem& texture_system);

        static Mesh load_obj(const std::string& path,
                             GPUMeshManager& mesh_manager,
                             MaterialSystem& material_system);
    };
}
