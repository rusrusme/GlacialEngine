//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/SimulationState.hpp"

namespace engine {
    void SimulationState::on_update(const float delta_time) {
        if (state_ == SceneState::Play) {
            simulation_time_ += delta_time;
        }
    }

    void SimulationState::play() {
        if (state_ == SceneState::Pause) {
            state_ = SceneState::Play;
            return;
        }
        state_ = SceneState::Play;
        simulation_time_ = 0.0f;
    }

    void SimulationState::pause() {
        if (state_ == SceneState::Play) {
            state_ = SceneState::Pause;
        }
    }

    void SimulationState::stop() {
        if (state_ == SceneState::Edit) return;

        state_ = SceneState::Edit;
        simulation_time_ = 0.0f;
    }

    SceneState SimulationState::state() const { return state_; }

    float SimulationState::time() const { return simulation_time_; }
}
