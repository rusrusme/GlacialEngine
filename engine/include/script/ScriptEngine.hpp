//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <atomic>
#include <filesystem>
#include <future>

#include "project/scene/ecs/ComponentRegistry.hpp"
#include "script/ScriptRegistry.hpp"

namespace engine {
    class Scene;

    class ScriptEngine {
    public:
        static void request_reload();
        static void on_update(Scene* active_scene);
        static bool compiling() { return s_compiling.load(); }

        static bool load_library(const std::filesystem::path& dll_path);
        static void unload_library();
        static bool reload(Scene* active_scene = nullptr);

        [[nodiscard]] static bool is_loaded() noexcept { return s_library_handle != nullptr; }

    private:
        using InitScriptsFun = void(*)(ScriptRegistry::ModuleList*, ComponentRegistry::CopierMap*);

        static int build_scripts();
        static void swap_library(Scene* active_scene);

        inline static std::atomic<bool> s_compiling{false};
        inline static std::future<int> s_build_task;

        inline static void* s_library_handle = nullptr;
        inline static std::filesystem::path s_original_path{};
        inline static std::filesystem::path s_live_path{};
    };
}