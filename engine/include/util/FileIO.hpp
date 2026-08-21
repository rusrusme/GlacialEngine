//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace engine::util {
    inline std::string read_file_to_string(const std::string& filepath) {
        if (!std::filesystem::exists(filepath)) {
            std::cerr << "Engine error: file: shader file does not exist: " << filepath << std::endl;
            return "";
        }

        const std::ifstream file(filepath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Engine error: file: failed to open shader file: " << filepath << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}
