//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/legacy/BSHLoader.hpp"

#include "core/Log.hpp"

#include <fstream>

#include <Compiler.hpp>
#include <Preprocessor.hpp>

namespace engine::legacy {
    using namespace engine;
    LoadedBSHData BSHLoader::load_from_file(const std::filesystem::path& path) {
        const std::string ext = path.extension().string();
        if (ext == ".bshb") {
            return load_from_binary(path);
        }
        if (ext == ".bsh") {
            // Check if up-to-date .bshb cached file exists
            std::filesystem::path binary_cache = path;
            binary_cache.replace_extension(".bshb");

            if (std::filesystem::exists(binary_cache) &&
                std::filesystem::last_write_time(binary_cache) >= std::filesystem::last_write_time(path)) {
                return load_from_binary(binary_cache);
            }

            return compile_from_source(path);
        }

        CORE_ERROR("Unsupported shader extension for BSHLoader: {0}", path.string());
        return {};
    }

    LoadedBSHData BSHLoader::load_from_binary(const std::filesystem::path& bshb_path) {
        LoadedBSHData result;

        std::ifstream file(bshb_path, std::ios::binary);
        if (!file.is_open()) {
            CORE_ERROR("Failed to open binary shader file: {0}", bshb_path.string());
            return result;
        }

        file.read(reinterpret_cast<char*>(&result.header), sizeof(bshc::BSHHeader));
        if (result.header.signature != bshc::BSH_SIGNATURE) {
            CORE_ERROR("Invalid BSH signature in file: {0}", bshb_path.string());
            return result;
        }

        for (uint8_t i = 0; i < result.header.stage_count; ++i) {
            bshc::BSHStageHeader stage_header{};
            file.read(reinterpret_cast<char*>(&stage_header), sizeof(bshc::BSHStageHeader));

            LoadedBSHStage stage{
                .stage_type = stage_header.stage_type,
                .is_spirv   = (stage_header.is_spirv != 0),
                .data       = std::vector<uint8_t>(stage_header.data_size)
            };

            file.read(reinterpret_cast<char*>(stage.data.data()), stage_header.data_size);
            result.stages.push_back(std::move(stage));
        }

        result.valid = true;
        return result;
    }

    LoadedBSHData BSHLoader::compile_from_source(const std::filesystem::path& bsh_path) {
        LoadedBSHData result;

        std::ifstream file(bsh_path);
        if (!file.is_open()) {
            CORE_ERROR("Failed to open source shader file: {0}", bsh_path.string());
            return result;
        }

        std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

        bshc::Preprocessor preprocessor(bsh_path.parent_path());
        bshc::ParseResult parse_result = preprocessor.process(source, bsh_path);

        if (!parse_result.success) {
            CORE_ERROR("BSH Preprocessing error in '{0}': {1}", bsh_path.string(), parse_result.error_message);
            return result;
        }

        bshc::Compiler compiler;
        auto [success, error_message, binary_data] = compiler.compile(parse_result, bsh_path.filename().string());

        if (!success) {
            CORE_ERROR("BSH Compilation error in '{0}': {1}", bsh_path.string(), error_message);
            return result;
        }

        // Save compiled binary cache to disk
        std::filesystem::path binary_path = bsh_path;
        binary_path.replace_extension(".bshb");
        if (std::ofstream out_binary(binary_path, std::ios::binary); out_binary.is_open()) {
            out_binary.write(reinterpret_cast<const char*>(binary_data.data()), binary_data.size());
        }

        // Load back from memory buffer
        return load_from_binary(binary_path);
    }
}
