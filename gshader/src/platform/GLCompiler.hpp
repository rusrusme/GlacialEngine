//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "gshader/Compiler.hpp"

namespace gshader {
    class GLCompiler : public Compiler {
    public:
        GLCompiler() = default;
        ~GLCompiler() override = default;

        CompilerOutput compile(const ParsedData& data) override;

    private:
        static std::string process_glsl_source(std::string_view raw_source, ShaderStage stage);
    };
}
