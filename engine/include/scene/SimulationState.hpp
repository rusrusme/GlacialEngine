//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "scene/Scene.hpp"
#include "scene/SceneState.hpp"

namespace engine {
    class SimulationState {
    public:
        SimulationState();
        ~SimulationState();

        void on_init(std::shared_ptr<Scene> scene);
        void on_shutdown();

        void on_update(float delta_time);

        void on_scene_play();
        void on_scene_pause();
        void on_scene_stop();

        [[nodiscard]] SceneState current_state() const;
        [[nodiscard]] std::shared_ptr<Scene> active_scene() const;
        [[nodiscard]] float simulation_time() const;

    private:
        SceneState scene_state_{SceneState::Edit};

        std::shared_ptr<Scene> scene_ = nullptr;
        std::shared_ptr<Scene> simulation_scene_ = nullptr;

        float simulation_time_ = 0.0f;
    };
}
