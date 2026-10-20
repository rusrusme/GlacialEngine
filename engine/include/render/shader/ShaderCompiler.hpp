//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glacialc/GShader.hpp>

#include "render/rhi/RHITypes.hpp"
#include "render/shader/GShaderLoader.hpp"

namespace engine {
    class ShaderCompiler {
    public:
        static LoadedShaderData compile_source(const std::string& source, const std::string& name);
        static LoadedShaderData compile_file(const std::filesystem::path& source);

    private:
        static glacialc::RenderBackend native_backend(RendererBackend backend);
    };
}
