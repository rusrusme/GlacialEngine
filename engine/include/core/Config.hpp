//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#include "core/Log.hpp"

namespace engine::config {
    constexpr auto ENGINE_TITLE = "Glacial Engine";

    namespace video {
        constexpr int MONITOR_RESOLUTION_WIDTH = 1920;
        constexpr int MONITOR_RESOLUTION_HEIGHT = 1080;

        constexpr float FPS_REFRESH_RATE_IN_SECONDS = 0.5f;

        inline bool IS_VSYNC = false;

        // --- Window Settings ---
        inline int WINDOW_WIDTH = 1600;
        inline int WINDOW_HEIGHT = 900;
        inline int WINDOW_X = -1; // -1 indicates default OS centered placement
        inline int WINDOW_Y = -1;
        inline bool IS_MAXIMIZED = false;

        inline int target_fps_index = 7;
        const int fps_presets[] = {30, 60, 90, 120, 144, 165, 240, INT32_MAX};
        constexpr int num_presets = std::size(fps_presets);

        inline int target_fps() {
            return fps_presets[target_fps_index];
        }

        inline std::string target_fps_str() {
            const int fps = target_fps();
            if (fps == INT32_MAX) {
                return "INF";
            }
            return std::to_string(fps);
        }
    }

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
                }
                else {
                    iss.clear();
                    std::string str_val;
                    if (iss >> str_val) {
                        video::IS_VSYNC = (str_val == "true" || str_val == "1");
                    }
                }
            }
            else if (key == "window_width") {
                iss >> video::WINDOW_WIDTH;
            }
            else if (key == "window_height") {
                iss >> video::WINDOW_HEIGHT;
            }
            else if (key == "window_x") {
                iss >> video::WINDOW_X;
            }
            else if (key == "window_y") {
                iss >> video::WINDOW_Y;
            }
            else if (key == "is_maximized") {
                int val;
                if (iss >> val) video::IS_MAXIMIZED = (val != 0);
            }
        }
        CORE_TRACE("Configuration settings loaded");
    }

    inline void save() {
        std::ofstream file(CONFIG_FILE_PATH);
        if (!file.is_open()) {
            CORE_ERROR("Failed to open '{0}' for writing config", CONFIG_FILE_PATH);
            return;
        }
        file << "target_fps_index " << video::target_fps_index << "\n";
        file << "is_vsync " << (video::IS_VSYNC ? 1 : 0) << "\n";
        file << "window_width " << video::WINDOW_WIDTH << "\n";
        file << "window_height " << video::WINDOW_HEIGHT << "\n";
        file << "window_x " << video::WINDOW_X << "\n";
        file << "window_y " << video::WINDOW_Y << "\n";
        file << "is_maximized " << (video::IS_MAXIMIZED ? 1 : 0) << "\n";

        CORE_TRACE("Configuration saved");
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
