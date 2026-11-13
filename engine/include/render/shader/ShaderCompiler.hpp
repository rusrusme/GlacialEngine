//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <gshader/GShader.hpp>

#include "render/rhi/RHITypes.hpp"
#include "render/shader/GShaderLoader.hpp"

namespace engine {
    class ShaderCompiler {
    public:
        static LoadedShaderData compile_source(const std::string& source, const std::string& name);
        static LoadedShaderData compile_file(const std::filesystem::path& source);

    private:
        static gshader::RenderBackend native_backend(RendererBackend backend);
    };
}
