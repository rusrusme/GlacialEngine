//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <glm/vec4.hpp>

#include "core/Log.hpp"

namespace editor::config {
    constexpr auto CONFIG_PATH = "editor.config";
    inline std::unordered_map<std::string, std::string> s_config_cache;
    inline bool s_is_loaded = false;

    inline void load() {
        s_config_cache.clear();
        std::ifstream file(CONFIG_PATH);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;

            std::istringstream iss(line);
            std::string key;
            if (iss >> key) {
                std::string value;
                std::getline(iss >> std::ws, value);
                if (!value.empty()) {
                    s_config_cache[key] = value;
                }
            }
        }
        s_is_loaded = true;
    }

    template <typename T>
    T get_value(const std::string& key, const T default_value = T{}) {
        if (!s_is_loaded) load();

        const auto it = s_config_cache.find(key);
        if (it == s_config_cache.end()) return default_value;

        try {
            if constexpr (std::is_same_v<T, int>) {
                return std::stoi(it->second);
            }
            else if constexpr (std::is_same_v<T, float>) {
                return std::stof(it->second);
            }
            else if constexpr (std::is_same_v<T, double>) {
                return std::stod(it->second);
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return (it->second == "true" || it->second == "1");
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                return it->second;
            }
            else if constexpr (std::is_same_v<T, glm::vec4>) {
                const char* ptr = it->second.c_str();
                char* end = nullptr;
                float components[4];
                int count = 0;

                while (count < 4 && *ptr != '\0') {
                    // Skip separators: spaces, commas, semicolons, tabs
                    while (*ptr == ' ' || *ptr == '\t' || *ptr == ',' || *ptr == ';') {
                        ++ptr;
                    }
                    if (*ptr == '\0') break;

                    components[count] = std::strtof(ptr, &end);
                    if (ptr == end) break; // Conversion stopped without consuming characters

                    ptr = end;
                    ++count;
                }

                if (count == 4) {
                    return glm::vec4(components[0], components[1], components[2], components[3]);
                }
                return default_value;
            }
        }
        catch (...) {
            return default_value;
        }

        return default_value;
    }

    template <typename T>
    void set_value(const std::string& key, const T& value) {
        if constexpr (std::is_same_v<T, std::string>) {
            s_config_cache[key] = value;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            s_config_cache[key] = value ? "true" : "false";
        }
        else if constexpr (std::is_same_v<T, glm::vec4>) {
            s_config_cache[key] = std::format("{},{},{},{}", value.x, value.y, value.z, value.w);
        }
        else {
            s_config_cache[key] = std::to_string(value);
        }
    }

    inline void save() {
        std::ofstream file(CONFIG_PATH);
        if (!file.is_open()) {
            CLIENT_ERROR("Failed to save '{0}'", CONFIG_PATH);
            return;
        }
        for (const auto& [key, value] : s_config_cache) {
            file << key << " " << value << "\n";
        }
    }
}