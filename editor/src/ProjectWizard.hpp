//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "project/Project.hpp"

namespace editor {
    class ProjectWizard {
    public:
        static bool setup(const engine::ProjectConfig& config,
                          const std::filesystem::path& template_dir = "templates/default");
        static bool setup(const std::shared_ptr<engine::Project>& project,
                          const std::filesystem::path& template_dir = "templates/default");
    };
}
