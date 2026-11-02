//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>

#include "project/scene/ecs/ComponentRegistry.hpp"
#include "script/ScriptRegistry.hpp"

#pragma once

namespace engine {
    class Scene;

    class ScriptEngine {
    public:
        static bool load_library(const std::filesystem::path& dll_path);
        static void unload_library();
        static bool reload(Scene* active_scene = nullptr);

        [[nodiscard]] static bool is_loaded() noexcept { return s_library_handle != nullptr; }

    private:
        using InitScriptsFun = void(*)(ScriptRegistry::ModuleList*, ComponentRegistry::CopierMap*);

        inline static void* s_library_handle = nullptr;
        inline static std::filesystem::path s_original_path{};
        inline static std::filesystem::path s_live_path{};
    };
}