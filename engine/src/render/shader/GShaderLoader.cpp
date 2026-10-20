//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/shader/GShaderLoader.hpp"

#include "render/rhi/RHITypes.hpp"

namespace engine {
    std::optional<LoadedShaderStage> LoadedShaderData::get_stage(const glacialc::ShaderStage stage_type) const {
        for (const auto& stage : stages) {
            if (stage.stage == stage_type) return stage;
        }
        return std::nullopt;
    }

    LoadedShaderData GShaderLoader::load_from_binary(const std::filesystem::path& gsb_path) {
        std::ifstream file(gsb_path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {.valid = false, .error_message = "Failed to open .gsb file: " + gsb_path.string()};
        }

        const auto file_size = file.tellg();
        if (file_size < static_cast<std::streamoff>(sizeof(glacialc::GShaderHeader))) {
            return {.valid = false, .error_message = "File is smaller than GShaderHeader size"};
        }

        Vector<uint8_t> buffer(file_size);
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer.data()), file_size);

        return load_from_memory(buffer);
    }

    LoadedShaderData GShaderLoader::load_from_memory(const std::span<const uint8_t> buffer) {
        LoadedShaderData result;

        if (buffer.size() < sizeof(glacialc::GShaderHeader)) {
            result.error_message = "Buffer smaller than GShaderHeader";
            return result;
        }

        glacialc::GShaderHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(glacialc::GShaderHeader));

        if (header.signature != glacialc::GSH_SIGNATURE) {
            result.error_message = "Invalid shader signature (magic number mismatch)";
            return result;
        }

        const size_t stage_headers_size = header.stage_count * sizeof(glacialc::GShaderStageHeader);
        const size_t min_required_size = sizeof(glacialc::GShaderHeader) + stage_headers_size;

        if (buffer.size() < min_required_size) {
            result.error_message = "Truncated shader file: missing stage headers";
            return result;
        }

        result.backend = header.backend;
        result.version = header.version;
        result.stages.reserve(header.stage_count);

        const uint8_t* stage_headers_ptr = buffer.data() + sizeof(glacialc::GShaderHeader);

        for (uint8_t i = 0; i < header.stage_count; ++i) {
            glacialc::GShaderStageHeader stage_hdr{};
            std::memcpy(&stage_hdr, stage_headers_ptr + (i * sizeof(glacialc::GShaderStageHeader)), sizeof(glacialc::GShaderStageHeader));

            const uint64_t payload_end = static_cast<uint64_t>(stage_hdr.data_offset) + stage_hdr.data_size;
            if (payload_end > buffer.size()) {
                result.error_message = "Stage payload offset/size out of buffer bounds";
                result.stages.clear();
                return result;
            }

            LoadedShaderStage stage;
            stage.stage = stage_hdr.stage_type;
            stage.code.resize(stage_hdr.data_size);

            std::memcpy(stage.code.data(), buffer.data() + stage_hdr.data_offset, stage_hdr.data_size);
            result.stages.push_back(std::move(stage));
        }
        result.valid = true;
        return result;
    }
}
