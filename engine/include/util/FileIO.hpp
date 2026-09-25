//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "core/Log.hpp"

namespace engine::util {
    inline std::string read_file_to_string(const std::string& filepath) {
        if (!std::filesystem::exists(filepath)) {
            CORE_ERROR("Failed reading file. Shader file does not exist: {0}", filepath.c_str());
            return "";
        }

        const std::ifstream file(filepath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            CORE_ERROR("Failed to open shader file: {0}", filepath.c_str());
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}
