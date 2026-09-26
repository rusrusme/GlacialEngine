//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <type_traits>

namespace editor::config {
    constexpr auto CONFIG_FILE_PATH = "editor.ini";
    inline std::unordered_map<std::string, std::string> s_config_cache;
    inline bool s_is_loaded = false;

    inline void load() {
        s_config_cache.clear();
        std::ifstream file(CONFIG_FILE_PATH);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;

            std::istringstream iss(line);
            std::string key, value;
            if (iss >> key >> value) {
                s_config_cache[key] = value;
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
        }
        catch (...) {
            return default_value; // Fallback if string conversion fails
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
        else {
            s_config_cache[key] = std::to_string(value);
        }
    }

    inline void save() {
        std::ofstream file(CONFIG_FILE_PATH);
        if (!file.is_open()) {
            CLIENT_ERROR("Failed to save '{0}'", CONFIG_FILE_PATH);
            return;
        }
        for (const auto& [key, value] : s_config_cache) {
            file << key << " " << value << "\n";
        }
    }
}