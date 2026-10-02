//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/Project.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

#include "core/Log.hpp"

using json = nlohmann::json;

namespace engine {
    std::shared_ptr<Project> Project::create_new(const std::filesystem::path& path) {
        const auto project = std::make_shared<Project>();

        project->config_.project_file_path = path;
        project->config_.project_directory = path.parent_path();
        project->config_.name = path.stem().string();

        // Set working directory to project folder
        std::filesystem::current_path(project->config_.project_directory);

        // Create standard directory layout
        std::filesystem::create_directories(get_asset_directory() / "scenes");
        std::filesystem::create_directories(get_asset_directory() / "textures");
        std::filesystem::create_directories(get_asset_directory() / "meshes");

        active_project_ = project;
        save_active();

        CORE_INFO("Created new project '{0}' at '{1}'", project->config_.name, path.string());
        return active_project_;
    }

    std::shared_ptr<Project> Project::load(const std::filesystem::path& path) {
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
        project->config_.initial_scene = proj_json.value("initial_scene", "assets/scenes/clean.scene");
        project->config_.asset_directory = proj_json.value("asset_directory", "assets");

        active_project_ = project;

        // Switch engine context to project directory
        std::filesystem::current_path(project->config_.project_directory);

        CORE_INFO("Successfully loaded project '{0}' from '{1}'", project->config_.name, path.string());
        return active_project_;
    }

    bool Project::save_active() {
        if (!active_project_) return false;
        return save_as(active_project_->config_.project_file_path);
    }

    bool Project::save_as(const std::filesystem::path& path) {
        if (!active_project_) return false;

        active_project_->config_.project_file_path = path;
        active_project_->config_.project_directory = path.parent_path();

        json root;
        root["project"] = {
            {"name", active_project_->config_.name},
            {"initial_scene", active_project_->config_.initial_scene.generic_string()},
            {"asset_directory", active_project_->config_.asset_directory.generic_string()}
        };

        std::ofstream out(path);
        if (!out.is_open()) {
            CORE_ERROR("Failed to save project: Could not open file for writing at '{0}'", path.string());
            return false;
        }

        out << root.dump(4);
        CORE_INFO("Successfully saved project '{0}' to '{1}'", active_project_->config_.name, path.string());
        return true;
    }
}
