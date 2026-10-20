//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/shader/ShaderCompiler.hpp"

#include <glacialc/GShaderCompiler.hpp>

#include "core/Log.hpp"
#include "render/rhi/RHIDevice.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    LoadedShaderData ShaderCompiler::compile_source(const std::string& source, const std::string& name) {
        const auto active_backend = native_backend(RHIDevice::backend());

        const std::vector<uint8_t> bin = glacialc::GShaderCompiler::compile_source(source, active_backend);
        if (bin.empty()) {
            return {.valid = false, .error_message = "Shader compilation failed for " + name };
        }
        return GShaderLoader::load_from_memory(bin);
    }

    LoadedShaderData ShaderCompiler::compile_file(const std::filesystem::path& source) {
        if (!std::filesystem::exists(source)) {
            CORE_ERROR("[ShaderCompiler] File does not exist: '{}'", source.string());
            return { .valid = false, .error_message = "File does not exist: " + source.string() };
        }

        std::ifstream file(source);
        if (!file.is_open()) {
            CORE_ERROR("[ShaderCompiler] Failed to open file: '{}'", source.string());
            return { .valid = false, .error_message = "Failed to open file: " + source.string() };
        }

        const std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
        const auto active_backend = native_backend(RHIDevice::backend());

        const std::vector<uint8_t> bin = glacialc::GShaderCompiler::compile_source(content, active_backend, source.parent_path());
        if (bin.empty()) {
            return { .valid = false, .error_message = "Shader compilation failed for " + source.string() };
        }
        return GShaderLoader::load_from_memory(bin);
    }

    glacialc::RenderBackend ShaderCompiler::native_backend(const RendererBackend backend) {
        switch (backend) {
            case RendererBackend::OpenGL: return glacialc::RenderBackend::OpenGL;
            case RendererBackend::Vulkan: return glacialc::RenderBackend::Vulkan;
            case RendererBackend::DX12:   return glacialc::RenderBackend::DX12;
            default:                      return glacialc::RenderBackend::OpenGL;
        }
    }
}
