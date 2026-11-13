//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <optional>
#include <string_view>

#include "GShader.hpp"

namespace gshader {
    constexpr const char* to_string(const RenderBackend& backend) {
        switch (backend) {
            case RenderBackend::OpenGL : return "OpenGL";
            case RenderBackend::Vulkan : return "Vulkan";
            case RenderBackend::DX12 : return "DX12";
            default : return "Unknown";
        }
    }

    inline std::optional<RenderBackend> parse_backend_string(const std::string_view str) {
        if (str == "opengl" || str == "gl") return RenderBackend::OpenGL;
        if (str == "vulkan" || str == "vk") return RenderBackend::Vulkan;
        if (str == "dx12" || str == "d3d12" || str == "directx12") return RenderBackend::DX12;
        return std::nullopt;
    }


}
