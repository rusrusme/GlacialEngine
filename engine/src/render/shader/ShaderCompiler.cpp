//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/shader/ShaderCompiler.hpp"

#include <bshc/BSHCompiler.hpp>

#include "core/Log.hpp"
#include "render/rhi/RHIDevice.hpp"
#include "render/rhi/RHITypes.hpp"

namespace engine {
    LoadedBSHData ShaderCompiler::compile_source(const std::string& source, const std::string& name) {
        const auto active_backend = native_backend(RHIDevice::backend());

        const std::vector<uint8_t> bin = bshc::BSHCompiler::compile_source(source, active_backend);
        if (bin.empty()) {
            return {.valid = false, .error_message = "Shader compilation failed for " + name };
        }
        return BSHLoader::load_from_memory(bin);
    }

    LoadedBSHData ShaderCompiler::compile_file(const std::filesystem::path& source) {
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

        const std::vector<uint8_t> bin = bshc::BSHCompiler::compile_source(content, active_backend, source.parent_path());
        if (bin.empty()) {
            return { .valid = false, .error_message = "Shader compilation failed for " + source.string() };
        }
        return BSHLoader::load_from_memory(bin);
    }

    bshc::RenderBackend ShaderCompiler::native_backend(const RendererBackend backend) {
        switch (backend) {
            case RendererBackend::OpenGL: return bshc::RenderBackend::OpenGL;
            case RendererBackend::Vulkan: return bshc::RenderBackend::Vulkan;
            case RendererBackend::DX12:   return bshc::RenderBackend::DX12;
            default:                      return bshc::RenderBackend::OpenGL;
        }
    }
}
