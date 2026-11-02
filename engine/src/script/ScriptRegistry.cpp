//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "script/ScriptRegistry.hpp"

namespace engine {
    void ScriptRegistry::add_script_module(ComponentRegisterFun comp_fn, SystemRegisterFun sys_fn) {
        auto& script_modules = modules();
        for (const auto& mod : script_modules) {
            if (mod.register_systems == sys_fn) {
                return; // Already registered
            }
        }
        script_modules.push_back({.register_components = comp_fn, .register_systems = sys_fn});
    }

    void ScriptRegistry::bind_scene(Scene& scene) {
        for (const auto& [register_comp, register_systems] : modules()) {
            if (register_comp) register_comp();
            if (register_systems) register_systems(scene);
        }
    }

    void ScriptRegistry::clear() { modules().clear(); }

    ScriptRegistry::ModuleList& ScriptRegistry::modules() {
        static ModuleList local_modules;
        return s_host_modules ? *s_host_modules : local_modules;
    }

    void ScriptRegistry::set_host_modules(ModuleList* host_modules) {
        s_host_modules = host_modules;
    }
}
