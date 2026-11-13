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

    // TODO Once you wire up Action::NewProject and Action::OpenProject in CoreLayer.cpp, you will just want to make sure:
    //      AssetManager::scan_directory(...) re-scans the loaded project's asset directory (so .meta handles are generated for project-specific textures and meshes)
    //      BuildScripts.cmd / ScriptEngine::reload() receives the active project's scripts path rather than only compiling ENGINE_DIR/assets/scripts
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
