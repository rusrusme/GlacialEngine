//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <utility>

#include "project/scene/Scene.hpp"

namespace engine {
    class SceneSerializer {
    public:
        explicit SceneSerializer(std::shared_ptr<Scene> scene) : scene_(std::move(scene)) {}

        // Disk file
        void serialize(const std::string& filepath, AssetManager& asset_manager);
        bool deserialize(const std::string& filepath, const SystemContext& context);

    private:
        std::shared_ptr<Scene> scene_;
    };
}
