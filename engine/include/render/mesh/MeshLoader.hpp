//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

#include "Mesh.hpp"

namespace engine {
    class MeshLoader {
    public:
        static Mesh load(const std::string& path);
    };
}
