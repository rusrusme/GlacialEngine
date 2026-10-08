//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/shader/BSHLoader.hpp"

namespace engine {
    std::optional<LoadedBSHStage> LoadedBSHData::get_stage(const bshc::ShaderStage stage_type) const {
        for (const auto& stage : stages) {
            if (stage.stage == stage_type) return stage;
        }
        return std::nullopt;
    }

    LoadedBSHData BSHLoader::load_from_binary(const std::filesystem::path& bshb_path) {
        std::ifstream file(bshb_path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {.valid = false, .error_message = "Failed to open BSHB file: " + bshb_path.string()};
        }

        const auto file_size = file.tellg();
        if (file_size < static_cast<std::streamoff>(sizeof(bshc::BSHHeader))) {
            return {.valid = false, .error_message = "File is smaller than BSHHeader size"};
        }

        std::vector<uint8_t> buffer(file_size);
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer.data()), file_size);

        return load_from_memory(buffer);
    }

    LoadedBSHData BSHLoader::load_from_memory(const std::span<const uint8_t> buffer) {
        LoadedBSHData result;

        if (buffer.size() < sizeof(bshc::BSHHeader)) {
            result.error_message = "Buffer smaller than BSHHeader";
            return result;
        }

        bshc::BSHHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(bshc::BSHHeader));

        if (header.signature != bshc::BSH_SIGNATURE) {
            result.error_message = "Invalid BSH signature (magic number mismatch)";
            return result;
        }

        const size_t stage_headers_size = header.stage_count * sizeof(bshc::BSHStageHeader);
        const size_t min_required_size = sizeof(bshc::BSHHeader) + stage_headers_size;

        if (buffer.size() < min_required_size) {
            result.error_message = "Truncated BSH file: missing stage headers";
            return result;
        }

        result.backend = header.backend;
        result.version = header.version;
        result.stages.reserve(header.stage_count);

        const uint8_t* stage_headers_ptr = buffer.data() + sizeof(bshc::BSHHeader);

        for (uint8_t i = 0; i < header.stage_count; ++i) {
            bshc::BSHStageHeader stage_hdr{};
            std::memcpy(&stage_hdr, stage_headers_ptr + (i * sizeof(bshc::BSHStageHeader)), sizeof(bshc::BSHStageHeader));

            const uint64_t payload_end = static_cast<uint64_t>(stage_hdr.data_offset) + stage_hdr.data_size;
            if (payload_end > buffer.size()) {
                result.error_message = "Stage payload offset/size out of buffer bounds";
                result.stages.clear();
                return result;
            }

            LoadedBSHStage stage;
            stage.stage = stage_hdr.stage_type;
            stage.code.resize(stage_hdr.data_size);

            std::memcpy(stage.code.data(), buffer.data() + stage_hdr.data_offset, stage_hdr.data_size);
            result.stages.push_back(std::move(stage));
        }
        result.valid = true;
        return result;
    }
}
