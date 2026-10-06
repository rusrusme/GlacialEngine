//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <BSH.hpp>

namespace engine::legacy {
    using namespace engine;
    struct LoadedBSHStage {
        bshc::ShaderStage stage_type;
        bool is_spirv = true;
        std::vector<uint8_t> data;
    };

    struct LoadedBSHData {
        bshc::BSHHeader header{};
        std::vector<LoadedBSHStage> stages;
        bool valid = false;
    };

    class BSHLoader {
    public:
        static LoadedBSHData load_from_file(const std::filesystem::path& path);
        static LoadedBSHData load_from_binary(const std::filesystem::path& bshb_path);
        static LoadedBSHData compile_from_source(const std::filesystem::path& bsh_path);
    };
}
