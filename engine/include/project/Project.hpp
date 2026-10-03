//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    struct ProjectConfig {
        std::string name{};
        std::filesystem::path project_file_path{};
        std::filesystem::path project_directory{};
        std::filesystem::path initial_scene{};
        std::filesystem::path asset_directory = "assets";
        std::filesystem::path scripts_directory = "scripts";
    };

    class Project {
    public:
        static std::shared_ptr<Project> load_project(const std::filesystem::path& path);
        static std::shared_ptr<Project> new_project(const std::filesystem::path& path);
        void save_project() const;
        [[nodiscard]] bool save_as_project(const std::filesystem::path& path) const;

        [[nodiscard]] std::filesystem::path resolve_virtual_path(const std::filesystem::path& virtual_path) const;

        [[nodiscard]] const ProjectConfig& get_config() const { return config_; }

    private:
        ProjectConfig config_;
    };
}
