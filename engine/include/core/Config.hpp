//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace engine::setting {
    constexpr auto FRAME_TITLE = "Engine";

    constexpr int MONITOR_RESOLUTION_WIDTH = 1920;
    constexpr int MONITOR_RESOLUTION_HEIGHT = 1080;

    inline int target_fps_index = 7;
    inline const int fps_presets[] = {30, 60, 90, 120, 144, 165, 240, INT32_MAX};
    inline constexpr int num_presets = std::size(fps_presets);

    inline int get_target_fps() {
        return fps_presets[target_fps_index];
    }

    inline std::string get_target_fps_str() {
        const int fps = get_target_fps();
        if (fps == INT32_MAX) {
            return "INF";
        }
        return std::to_string(fps);
    }

    namespace config {
        constexpr auto CONFIG_FILE_PATH = "config.ini";

        inline void load() {
            std::ifstream file(CONFIG_FILE_PATH);
            if (!file.is_open()) {
                return;
            }
            std::string key;
            int value;
            while (file >> key >> value) {
                if (key == "target_fps_index") {
                    if (value >= 0 && value < num_presets) {
                        target_fps_index = value;
                    }
                }
            }
        }

        inline void save() {
            std::ofstream file(CONFIG_FILE_PATH);
            if (!file.is_open()) {
                std::cerr << "Engine error: save config: failed to open " << CONFIG_FILE_PATH << " for writing." << std::endl;
                return;
            }
            file << "target_fps_index " << target_fps_index << "\n";
        }
    }

    namespace camera {
        constexpr float DEFAULT_DISTANCE = 80.0f;
        constexpr float DEFAULT_YAW = 45.0f;
        constexpr float DEFAULT_PITCH = 60.0f;
    }

    namespace camera_controller {
        constexpr float SCROLL_SPEED = 1.0f;
        constexpr float ORBIT_SENSITIVITY = 0.25;
        constexpr float PAN_SENSITIVITY = 1.3f;
        constexpr float MIN_PAN_DISTANCE = 1.0f;
        constexpr float ZOOM_PERCENTAGE = 0.10f;
    }
}
