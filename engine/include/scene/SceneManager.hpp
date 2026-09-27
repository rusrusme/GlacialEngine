//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "scene/Scene.hpp"

namespace engine {
    class SceneManager {
    public:
        std::shared_ptr<Scene> active_scene();
    };
}
