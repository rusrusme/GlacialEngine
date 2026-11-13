
//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "Compiler.hpp"

namespace gshader {
    class VulkanCompiler : public Compiler {
    public:
        VulkanCompiler() = default;
        ~VulkanCompiler() override = default;

        CompilerOutput compile(const ParsedData& data) override;
    };
}
