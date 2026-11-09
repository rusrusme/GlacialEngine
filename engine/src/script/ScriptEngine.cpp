//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <chrono>

#include "script/ScriptEngine.hpp"
#include "core/Log.hpp"
#include "project/scene/Scene.hpp"
#include "util/Instrumentor.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace engine {
    void ScriptEngine::on_update(Scene* active_scene) {
        if (!s_compiling.load()) {
            return;
        }

        if (!s_build_task.valid()) {
            s_compiling.store(false);
            return;
        }

        // Non-blocking check
        if (s_build_task.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            int result = s_build_task.get();
            s_compiling.store(false);

            if (result != 0) {
                CORE_ERROR("[ScriptEngine] Compilation failed (exit code: {})", result);
                return;
            }
            swap_library(active_scene);
        }
    }

    bool ScriptEngine::load_library(const std::filesystem::path& dll_path) {
        PROFILE_FUNCTION();

        if (s_library_handle != nullptr) {
            unload_library();
        }

        if (!std::filesystem::exists(dll_path)) {
            CORE_ERROR("[ScriptEngine] DLL not found: {}", dll_path.string());
            return false;
        }

        s_original_path = dll_path;

        // Shadow copy
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
        auto init_scripts_fun = reinterpret_cast<InitScriptsFun>(GetProcAddress(static_cast<HMODULE>(s_library_handle), "glaze_init_scripts"));
        #else
        s_library_handle = dlopen(s_live_path.string().c_str(), RTLD_NOW);
        if (!s_library_handle) {
            CORE_ERROR("[ScriptEngine] dlopen failed: {}", dlerror());
            return false;
        }
        auto init_scripts_fun = reinterpret_cast<InitScriptsFun>(dlsym(s_library_handle, "glaze_init_scripts"));
        #endif

        if (!init_scripts_fun) {
            CORE_ERROR("[ScriptEngine] Entry point 'glaze_init_scripts' not found in DLL");
            unload_library();
            return false;
        }

        init_scripts_fun(&ScriptRegistry::modules(), &ComponentRegistry::get_copiers());
        CORE_INFO("[ScriptEngine] Successfully loaded script library: {}", s_original_path.filename().string());
        return true;
    }

    void ScriptEngine::unload_library() {
        PROFILE_FUNCTION();

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

    void ScriptEngine::reload() {
        if (s_compiling.load()) {
            CORE_WARN("[ScriptEngine] Compilation is already in progress!");
            return;
        }

        s_compiling.store(true);
        CORE_INFO("[ScriptEngine] Reloading scripts (background build started)...");

        s_build_task = std::async(std::launch::async, [] {
            return build_scripts();
        });
    }

    int ScriptEngine::build_scripts() {
        PROFILE_FUNCTION();

        #if defined(_WIN32)
        return std::system("BuildScripts.cmd");
        #else
        return std::system("./BuildScripts.sh");
        #endif
    }

    void ScriptEngine::swap_library(Scene* active_scene) {
        PROFILE_FUNCTION();

        CORE_INFO("[ScriptEngine] Applying new script library...");

        if (active_scene) {
            active_scene->on_shutdown();
        }

        unload_library();

        if (!load_library(s_original_path)) {
            CORE_ERROR("[ScriptEngine] Failed to reload script library: {}", s_original_path.string());
            return;
        }

        if (active_scene) {
            active_scene->on_init();
        }

        CORE_INFO("[ScriptEngine] Reload complete!");
    }
}
