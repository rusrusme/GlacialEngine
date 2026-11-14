//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <memory>

#include "render/mesh/Mesh.hpp"
#include "render/rhi/RHIDevice.hpp"

namespace engine {
    class MeshLoader {
    public:
        [[nodiscard]] static std::shared_ptr<Mesh> load(const RHIDevice& device, const std::filesystem::path& filepath);

    private:
        [[nodiscard]] static std::shared_ptr<Mesh> load_gltf(const RHIDevice& device, const std::filesystem::path& filepath);
    };
}
