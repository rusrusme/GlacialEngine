//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cctype>
#include <memory>
#include <string_view>
#include <shaderc/shaderc.h>

#include "gshader/GShader.hpp"

namespace gshader {
    template<typename T>
    using Shared = std::shared_ptr<T>;

    template<typename T>
    using Unique = std::unique_ptr<T>;

    inline std::string_view trim(std::string_view sv) {
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front()))) {
            sv.remove_prefix(1);
        }
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back()))) {
            sv.remove_suffix(1);
        }
        return sv;
    }

    inline shaderc_shader_kind to_shaderc_stage(const ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex : return shaderc_glsl_vertex_shader;
            case ShaderStage::Fragment : return shaderc_glsl_fragment_shader;
            case ShaderStage::Geometry : return shaderc_glsl_geometry_shader;
            case ShaderStage::Compute : return shaderc_glsl_compute_shader;
        }
        return shaderc_glsl_infer_from_source;
    }
}