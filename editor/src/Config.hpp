//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace editor::config {
    constexpr auto CONFIG_FILE_PATH = "editor.ini";
    inline std::unordered_map<std::string, int> s_config_cache;
    inline bool s_is_loaded = false;

    inline void load() {
        s_config_cache.clear();
        std::ifstream file(CONFIG_FILE_PATH);
        if (!file.is_open()) return;

        std::string key;
        int value = 0;
        while (file >> key >> value) {
            s_config_cache[key] = value;
        }
        s_is_loaded = true;
    }

    inline int get_value(const std::string& key, const int default_value = -1) {
        if (!s_is_loaded) load();
        const auto it = s_config_cache.find(key);
        return (it != s_config_cache.end()) ? it->second : default_value;
    }

    inline void set_value(const std::string& key, const int value) {
        s_config_cache[key] = value;
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
