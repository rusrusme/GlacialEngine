//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

#include "render/mesh/Mesh.hpp"

namespace engine {
    class MeshLoader {
    public:
        static Mesh load(const std::string& path);
    };
}
