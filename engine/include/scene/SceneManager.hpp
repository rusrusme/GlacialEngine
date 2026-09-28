//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "scene/Scene.hpp"
#include "scene/SceneState.hpp"
#include "scene/SimulationState.hpp"

namespace engine {
    class SceneManager {
    public:
        void on_attach(std::shared_ptr<Scene> scene);
        void on_detach();

        // --- Scene Management ---
        std::shared_ptr<Scene> create(const std::string& name = "Untitled Scene");

        void on_update(float delta_time);

        void on_play();
        void on_pause();
        void on_stop();

        [[nodiscard]] std::shared_ptr<Scene> active_scene() const;
        [[nodiscard]] SceneState scene_state() const;
        [[nodiscard]] float simulation_time() const;

    private:
        std::shared_ptr<Scene> scene_ = nullptr;
        std::shared_ptr<Scene> simulation_scene_ = nullptr;

        SimulationState simulation_state_{};
    };
}
