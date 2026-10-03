//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/Project.hpp"

#include <nlohmann/json.hpp>

#include "core/Log.hpp"

using json = nlohmann::json;

namespace engine {
    std::shared_ptr<Project> Project::load_project(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            CORE_ERROR("Failed to load project: File does not exist at '{0}'", path.string());
            return nullptr;
        }

        std::ifstream in(path);
        if (!in.is_open()) {
            CORE_ERROR("Failed to load project: Could not open file '{0}'", path.string());
            return nullptr;
        }

        json root;
        try {
            in >> root;
        }
        catch (const json::parse_error& e) {
            CORE_ERROR("Failed to parse project file '{0}': {1}", path.string(), e.what());
            return nullptr;
        }

        if (!root.contains("project") || !root["project"].is_object()) {
            CORE_ERROR("Failed to load project '{0}': Missing valid 'project' object", path.string());
            return nullptr;
        }

        const auto& proj_json = root["project"];
        auto project = std::make_shared<Project>();

        project->config_.project_file_path = path;
        project->config_.project_directory = path.parent_path();

        project->config_.name = proj_json.value("name", path.stem().string());
        project->config_.initial_scene = proj_json.value("initial_scene", "");
        project->config_.asset_directory = proj_json.value("asset_directory", "assets");
        project->config_.scripts_directory = proj_json.value("scripts_directory", "scripts");

        CORE_INFO("Successfully loaded project '{0}' from '{1}'", project->config_.name, path.string());
        return project;
    }

    std::shared_ptr<Project> Project::new_project(const std::filesystem::path& path) {
        auto project = std::make_shared<Project>();

        project->config_.project_file_path = path;
        project->config_.project_directory = path.parent_path();
        project->config_.name = path.stem().string();
        project->config_.initial_scene = "";

        const auto& proj_dir = project->config_.project_directory;

        std::filesystem::create_directories(proj_dir / project->config_.asset_directory / "scenes");
        std::filesystem::create_directories(proj_dir / project->config_.asset_directory / "models");
        std::filesystem::create_directories(proj_dir / project->config_.asset_directory / "fonts");
        std::filesystem::create_directories(proj_dir / project->config_.asset_directory / "audio");
        std::filesystem::create_directories(proj_dir / project->config_.scripts_directory);

        project->save_project();

        CORE_INFO("Created new project '{0}' at '{1}'", project->config_.name, path.string());
        return project;
    }

    void Project::save_project() const {
        if (!save_as_project(config_.project_file_path)) {
            CORE_ERROR("Failed to save current project");
        }
    }

    bool Project::save_as_project(const std::filesystem::path& path) const {
        json root;
        root["project"] = {
            {"name", config_.name},
            {"initial_scene", config_.initial_scene.generic_string()},
            {"asset_directory", config_.asset_directory.generic_string()},
            {"scripts_directory", config_.scripts_directory.generic_string()}
        };

        std::ofstream out(path);
        if (!out.is_open()) {
            CORE_ERROR("Failed to save project: Could not open file for writing at '{0}'", path.string());
            return false;
        }

        out << root.dump(4);
        CORE_INFO("Successfully saved project '{0}' to '{1}'", config_.name, path.string());
        return true;
    }

    std::filesystem::path Project::resolve_virtual_path(const std::filesystem::path& virtual_path) const {
        const std::string path_str = virtual_path.generic_string();
        if (path_str.rfind("assets://", 0) == 0) {
            return config_.project_directory / config_.asset_directory / path_str.substr(9);
        }
        return config_.project_directory / virtual_path;
    }
}
