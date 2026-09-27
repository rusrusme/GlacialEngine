//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <utility>

#include "scene/SimulationState.hpp"

#include "core/Log.hpp"

namespace engine {
    SimulationState::SimulationState() = default;
    SimulationState::~SimulationState() = default;

    void SimulationState::on_init(std::shared_ptr<Scene> scene) {
        scene_ = std::move(scene);
        if (scene_) {
            scene_->on_init();
        }
    }

    void SimulationState::on_shutdown() {
        on_scene_stop();
        scene_->on_shutdown();
        scene_.reset();
    }

    void SimulationState::on_update(const float delta_time) {
        if (scene_state_ == SceneState::Play) {
            simulation_time_ += delta_time;
            if (simulation_scene_) {
                simulation_scene_->on_update(delta_time);
            }
        }
    }

    void SimulationState::on_scene_play() {
        if (scene_state_ == SceneState::Pause) {
            scene_state_ = SceneState::Play;
            return;
        }
        scene_state_ = SceneState::Play;
        simulation_time_ = 0.0f;
        CORE_TRACE("Started scene simulation");

        simulation_scene_ = Scene::copy(scene_);
        if (simulation_scene_) {
            simulation_scene_->on_init();
        }
    }

    void SimulationState::on_scene_pause() {
        if (scene_state_ == SceneState::Play) {
            scene_state_ = SceneState::Pause;
            CORE_TRACE("Paused scene simulation");
        }
    }

    void SimulationState::on_scene_stop() {
        if (scene_state_ == SceneState::Edit) return;

        scene_state_ = SceneState::Edit;
        simulation_time_ = 0.0f;

        if (simulation_scene_) {
            simulation_scene_->on_shutdown();
            simulation_scene_.reset();
        }
        CORE_TRACE("Stopped scene simulation");
    }

    SceneState SimulationState::current_state() const { return scene_state_; }

    std::shared_ptr<Scene> SimulationState::active_scene() const {
        return scene_state_ == SceneState::Edit ? scene_ : simulation_scene_;
    }

    float SimulationState::simulation_time() const { return simulation_time_; }
}
