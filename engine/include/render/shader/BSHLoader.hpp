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
#include <vector>

#include <bshc/BSH.hpp>

namespace engine {
    struct LoadedBSHStage {
        bshc::ShaderStage stage;
        std::vector<uint8_t> code;

        [[nodiscard]] std::string_view as_string_view() const { return {reinterpret_cast<const char*>(code.data()), code.size()}; }
    };

    struct LoadedBSHData {
        bshc::RenderBackend backend = bshc::RenderBackend::OpenGL;
        uint16_t version = 1;
        std::vector<LoadedBSHStage> stages;

        bool valid = false;
        std::string error_message;

        [[nodiscard]] std::optional<LoadedBSHStage> get_stage(bshc::ShaderStage stage_type) const;
    };

    class BSHLoader {
    public:
        static LoadedBSHData load_from_binary(const std::filesystem::path& bshb_path);
        static LoadedBSHData load_from_memory(std::span<const uint8_t> buffer);
    };
} // engine
