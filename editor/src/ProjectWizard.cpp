//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ProjectWizard.hpp"

namespace {
    bool copy_template(const std::filesystem::path& template_dir, const std::filesystem::path& project_dir) {
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

    bool generate_cmake(const std::filesystem::path& template_cmake,
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

    bool copy_engine(const std::filesystem::path& engine_lib_dir, const std::filesystem::path& project_dir) {
        if (!std::filesystem::exists(engine_lib_dir)) {
            CORE_ERROR("ProjectWizard: 'engine-lib' directory not found at '{0}'", engine_lib_dir.string());
            return false;
        }
        const auto basis_dir = project_dir / "basis";
        std::error_code ec;

        std::filesystem::create_directories(basis_dir, ec);
        if (ec) {
            CORE_ERROR("ProjectWizard: Failed to create glacial directory '{0}': {1}", basis_dir.string(), ec.message());
            return false;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 engine_lib_dir,
                 std::filesystem::directory_options::skip_permission_denied,
                 ec)) {
            const auto& src_path = entry.path();

            const auto relative_path = std::filesystem::relative(src_path, engine_lib_dir, ec);
            if (ec) continue;

            if (!relative_path.empty() && *relative_path.begin() == "build") {
                continue;
            }

            const auto dest_path = basis_dir / relative_path;

            if (entry.is_directory()) {
                std::filesystem::create_directories(dest_path, ec);
            }
            else if (entry.is_regular_file()) {
                std::filesystem::create_directories(dest_path.parent_path(), ec);
                std::filesystem::copy_file(
                    src_path,
                    dest_path,
                    std::filesystem::copy_options::overwrite_existing,
                    ec
                );

                if (ec) {
                    CORE_WARN("ProjectWizard: Could not copy '{0}': {1}", relative_path.string(), ec.message());
                    ec.clear();
                }
            }
        }
        return true;
    }
}

namespace editor {
    bool ProjectWizard::setup(const engine::ProjectConfig& config, const std::filesystem::path& template_dir) {
        const auto absolute_template_dir = std::filesystem::path(ENGINE_DIR) / template_dir;

        if (!std::filesystem::exists(absolute_template_dir)) {
            CORE_ERROR("ProjectWizard: Template directory does not exist at '{0}'", absolute_template_dir.string());
            return false;
        }

        const auto engine_lib_dir = std::filesystem::path(ENGINE_DIR) / "engine-lib";
        const auto project_dir = config.project_directory;

        if (!copy_engine(engine_lib_dir, project_dir)) {
            return false;
        }

        const auto template_cmake = absolute_template_dir / "CMakeLists.txt";
        if (!generate_cmake(template_cmake, project_dir, config.name)) {
            return false;
        }
        if (!copy_template(absolute_template_dir, project_dir)) {
            return false;
        }

        CORE_INFO("Successfully scaffolded project '{0}' from templates at '{1}'", config.name, absolute_template_dir.string());
        return true;
    }

    bool ProjectWizard::setup(const std::shared_ptr<engine::Project>& project, const std::filesystem::path& template_dir) {
        if (!project) return false;
        return setup(project->get_config(), template_dir);
    }
}
