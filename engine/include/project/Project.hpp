//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    struct ProjectConfig {
        std::string name = "Untitled Project";
        std::filesystem::path initial_scene = "assets/scenes/clean.scene";
        std::filesystem::path asset_directory = "assets";
        std::filesystem::path script_module_path = "bin/GameScripts.dll";

        // Resolved at runtime based on where the .basis file is saved/loaded
        std::filesystem::path project_directory;
        std::filesystem::path project_file_path;
    };

    class Project {
    public:
        static std::shared_ptr<Project> get_active() { return active_project_; }

        static std::shared_ptr<Project> create_new(const std::filesystem::path& path);
        static std::shared_ptr<Project> load(const std::filesystem::path& path);
        static bool save_active();
        static bool save_as(const std::filesystem::path& path);

        [[nodiscard]] const ProjectConfig& get_config() const { return config_; }
        ProjectConfig& get_config() { return config_; }

        // Helper path resolvers
        static std::filesystem::path get_project_directory() {
            return active_project_ ? active_project_->config_.project_directory : std::filesystem::current_path();
        }

        static std::filesystem::path get_asset_directory() {
            return get_project_directory() / (active_project_ ? active_project_->config_.asset_directory : "assets");
        }

        static std::filesystem::path get_asset_file_path(const std::filesystem::path& relative_path) {
            return get_asset_directory() / relative_path;
        }

    private:
        ProjectConfig config_;
        inline static std::shared_ptr<Project> active_project_ = nullptr;
    };
}
