//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <utility>

#include "scene/Scene.hpp"

namespace engine {
    class SceneSerializer {
    public:
        explicit SceneSerializer(std::shared_ptr<Scene> scene) : scene_(std::move(scene)) {}

        void serialize(const std::string& filepath);
        bool deserialize(const std::string& filepath);

    private:
        std::shared_ptr<Scene> scene_;
    };
}
