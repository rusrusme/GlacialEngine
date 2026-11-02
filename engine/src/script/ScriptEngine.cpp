//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <chrono>

#include "script/ScriptEngine.hpp"
#include "core/Log.hpp"
#include "project/scene/Scene.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {
    int build_scripts() {
        #if defined(_WIN32)
        return std::system("BuildScripts.cmd");
        #else // linux
        return std::system("cmake --build .. --target Scripts")
        #endif
    }
}

namespace engine {
    static uint32_t s_reload_counter = 0;

    bool ScriptEngine::load_library(const std::filesystem::path& dll_path) {
        if (s_library_handle != nullptr) {
            unload_library();
        }

        if (!std::filesystem::exists(dll_path)) {
            CORE_ERROR("[ScriptEngine] DLL not found: {}", dll_path.string());
            return false;
        }

        s_original_path = dll_path;

        auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        s_live_path = dll_path.parent_path() / std::format("{}_live_{}{}",
                                                           dll_path.stem().string(),
                                                           timestamp,
                                                           dll_path.extension().string());

        try {
            std::filesystem::copy_file(s_original_path, s_live_path, std::filesystem::copy_options::overwrite_existing);
        }
        catch (const std::exception& e) {
            CORE_ERROR("[ScriptEngine] Failed to create shadow copy: {}", e.what());
            return false;
        }

        #if defined(_WIN32)
        s_library_handle = LoadLibraryA(s_live_path.string().c_str());
        if (!s_library_handle) {
            CORE_ERROR("[ScriptEngine] LoadLibrary failed. Error code: {}", GetLastError());
            return false;
        }
        auto init_fn = reinterpret_cast<InitFun>(GetProcAddress(static_cast<HMODULE>(s_library_handle), "glaze_init_scripts"));
        #else
        s_library_handle = dlopen(s_live_path.string().c_str(), RTLD_NOW);
        if (!s_library_handle) {
            CORE_ERROR("[ScriptEngine] dlopen failed: {}", dlerror());
            return false;
        }
        auto init_fn = reinterpret_cast<InitFun>(dlsym(s_library_handle, "glaze_init_scripts"));
        #endif

        if (!init_fn) {
            CORE_ERROR("[ScriptEngine] Entry point 'glaze_init_scripts' not found in DLL");
            unload_library();
            return false;
        }

        init_fn(&ScriptRegistry::get_modules(), &ComponentRegistry::get_copiers());
        CORE_INFO("[ScriptEngine] Successfully loaded script library: {}", s_original_path.filename().string());
        return true;
    }

    void ScriptEngine::unload_library() {
        if (!s_library_handle) return;

        ScriptRegistry::clear();

        #if defined(_WIN32)
        FreeLibrary(static_cast<HMODULE>(s_library_handle));
        #else
        dlclose(s_library_handle);
        #endif

        s_library_handle = nullptr;

        std::error_code ec;
        std::filesystem::remove(s_live_path, ec);

        CORE_INFO("[ScriptEngine] Unloaded script library");
    }

    bool ScriptEngine::reload(Scene* active_scene) {
        CORE_INFO("[ScriptEngine] Reloading scripts...");

        int result = build_scripts();
        if (result != 0) {
            CORE_ERROR("[ScriptEngine] Compilation failed");
            return false;
        }

        if (active_scene) {
            active_scene->on_shutdown();
        }
        unload_library();

        if (!load_library(s_original_path)) {
            return false;
        }

        if (active_scene) {
            active_scene->on_init();
        }

        CORE_INFO("[ScriptEngine] Reload complete!");
        return true;
    }
}
