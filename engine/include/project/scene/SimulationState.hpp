//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "project/scene/SceneState.hpp"

namespace engine {
    class SimulationState {
    public:
        SimulationState() = default;;
        ~SimulationState() = default;;

        void on_update(float delta_time);

        void play();
        void pause();
        void stop();

        [[nodiscard]] SceneState state() const;
        [[nodiscard]] float time() const;

    private:
        SceneState state_{SceneState::Edit};

        float simulation_time_ = 0.0f;
    };
}
