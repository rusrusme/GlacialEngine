//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include <glacialc/GShader.hpp>
#include "render/rhi/RHITypes.hpp"

namespace engine {
    struct LoadedShaderStage {
        glacialc::ShaderStage stage;
        Vector<uint8_t> code;

        [[nodiscard]] std::string_view as_string_view() const { return {reinterpret_cast<const char*>(code.data()), code.size()}; }
    };

    struct LoadedShaderData {
        glacialc::RenderBackend backend = glacialc::RenderBackend::OpenGL;
        uint16_t version = 1;
        Vector<LoadedShaderStage> stages;

        bool valid = false;
        std::string error_message;

        [[nodiscard]] std::optional<LoadedShaderStage> get_stage(glacialc::ShaderStage stage_type) const;
    };

    class GShaderLoader {
    public:
        static LoadedShaderData load_from_binary(const std::filesystem::path& gsb_path);
        static LoadedShaderData load_from_memory(std::span<const uint8_t> buffer);
    };
} // engine
