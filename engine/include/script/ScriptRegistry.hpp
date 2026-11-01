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

        static void add_script_module(ComponentRegisterFun comp_fn, SystemRegisterFun sys_fn);
        static void bind_scene(Scene& scene);

    private:
        struct ScriptModule {
            ComponentRegisterFun register_components;
            SystemRegisterFun register_systems;
        };

        static std::vector<ScriptModule>& get_modules();
    };
}
