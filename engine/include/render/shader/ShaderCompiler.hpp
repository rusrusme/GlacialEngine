//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <bshc/BSH.hpp>

#include "render/rhi/RHITypes.hpp"
#include "render/shader/BSHLoader.hpp"

namespace engine {
    class ShaderCompiler {
    public:
        static LoadedBSHData compile_source(const std::string& source, const std::string& name);
        static LoadedBSHData compile_file(const std::filesystem::path& source);

    private:
        static bshc::RenderBackend native_backend(RendererBackend backend);
    };
}
