//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <vector>

namespace engine {
    class Scene;

    class ScriptRegistry {
    public:
        using SystemRegisterFun = void(*)(Scene&);
        using ComponentRegisterFun = void(*)();

        struct ScriptModule {
            ComponentRegisterFun register_components;
            SystemRegisterFun register_systems;
        };

        using ModuleList = std::vector<ScriptModule>;

        static void add_script_module(ComponentRegisterFun comp_fn, SystemRegisterFun sys_fn);
        static void bind_scene(Scene& scene);
        static void clear();

        static ModuleList& get_modules();
        static void set_host_modules(ModuleList* host_modules);

    private:
        inline static ModuleList* s_host_modules = nullptr;
    };
}
