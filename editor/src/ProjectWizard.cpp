//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ProjectWizard.hpp"

namespace {
    bool copy_template_sources(const std::filesystem::path& template_dir, const std::filesystem::path& project_dir) {
        const auto src_target_dir = project_dir / "src";
        std::filesystem::create_directories(src_target_dir);

        // Copy all starter source/header files into project's src/ directory
        for (const auto& entry : std::filesystem::directory_iterator(template_dir)) {
            if (entry.is_regular_file()) {
                const auto ext = entry.path().extension();
                if (ext == ".cpp" || ext == ".hpp" || ext == ".h") {
                    const auto destination = src_target_dir / entry.path().filename();
                    if (!std::filesystem::exists(destination)) {
                        std::filesystem::copy_file(entry.path(), destination);
                    }
                }
            }
        }
        return true;
    }

    bool generate_cmake_from_template(const std::filesystem::path& template_cmake,
                                      const std::filesystem::path& project_dir,
                                      const std::string& project_name) {
        if (!std::filesystem::exists(template_cmake)) {
            CORE_ERROR("Template CMakeLists.txt not found at '{0}'", template_cmake.string());
            return false;
        }

        std::ifstream in(template_cmake);
        if (!in.is_open()) return false;

        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string cmake_content = buffer.str();

        // Replace ${PROJECT_NAME} placeholder with actual chosen project name
        const std::string placeholder = "${PROJECT_NAME}";
        size_t pos = 0;
        while ((pos = cmake_content.find(placeholder, pos)) != std::string::npos) {
            cmake_content.replace(pos, placeholder.length(), project_name);
            pos += project_name.length();
        }

        const auto target_cmake = project_dir / "CMakeLists.txt";
        std::ofstream out(target_cmake);
        if (!out.is_open()) return false;

        out << cmake_content;
        return true;
    }
}

namespace editor {
    bool ProjectWizard::setup(const engine::ProjectConfig& config, const std::filesystem::path& template_dir) {
        if (!std::filesystem::exists(template_dir)) {
            CORE_ERROR("ProjectWizard: Template directory does not exist at '{0}'", template_dir.string());
            return false;
        }

        const auto project_dir = config.project_directory;

        const auto template_cmake = template_dir / "CMakeLists.txt";
        if (!generate_cmake_from_template(template_cmake, project_dir, config.name)) {
            return false;
        }

        if (!copy_template_sources(template_dir, project_dir)) {
            return false;
        }

        CORE_INFO("Successfully scaffolded project '{0}' from templates at '{1}'", config.name, template_dir.string());
        return true;
    }

    bool ProjectWizard::setup(const std::shared_ptr<engine::Project>& project, const std::filesystem::path& template_dir) {
        if (!project) return false;
        return setup(project->get_config(), template_dir);
    }
}
