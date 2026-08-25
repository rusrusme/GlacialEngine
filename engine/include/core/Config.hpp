//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace engine::setting {
    constexpr auto FRAME_TITLE = "Engine";

    namespace video {
        constexpr int MONITOR_RESOLUTION_WIDTH = 1920;
        constexpr int MONITOR_RESOLUTION_HEIGHT = 1080;

        constexpr float FPS_REFRESH_RATE_IN_SECONDS = 0.5f;

        inline bool IS_VSYNC = false;

        inline int target_fps_index = 7;
        const int fps_presets[] = {30, 60, 90, 120, 144, 165, 240, INT32_MAX};
        constexpr int num_presets = std::size(fps_presets);

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
    }

    namespace configuration {
        constexpr auto CONFIG_FILE_PATH = "config.ini";

        inline void load() {
            std::ifstream file(CONFIG_FILE_PATH);
            if (!file.is_open()) return;

            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#' || line[0] == ';') continue;

                std::istringstream iss(line);
                std::string key;
                if (!(iss >> key)) continue;

                if (key == "target_fps_index") {
                    int value;
                    if (iss >> value && value >= 0 && value < video::num_presets) {
                        video::target_fps_index = value;
                    }
                }
                else if (key == "is_vsync") {
                    int int_val;
                    if (iss >> int_val) {
                        video::IS_VSYNC = (int_val != 0);
                    } else {
                        iss.clear();
                        std::string str_val;
                        if (iss >> str_val) {
                            video::IS_VSYNC = (str_val == "true" || str_val == "1");
                        }
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
            file << "target_fps_index " << video::target_fps_index << "\n";
            file << "is_vsync " << (video::IS_VSYNC ? 1 : 0) << "\n";
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
