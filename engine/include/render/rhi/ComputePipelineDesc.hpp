//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "render/rhi/Shader.hpp"

namespace engine {
    struct ComputePipelineDesc {
        Shared<Shader> shader{nullptr};
    };
}