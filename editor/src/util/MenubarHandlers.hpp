//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string>

#include "core/Application.hpp"

namespace editor {
    using namespace engine;

    inline void update_window_title(const std::string& path) {
        const std::string scene_name = std::filesystem::path(path).stem().string();
        Application::get().window().set_title(std::string(config::ENGINE_TITLE) + " - " + scene_name);
    }
}