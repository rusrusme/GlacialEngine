//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "scene/SceneManager.hpp"

#include "core/Log.hpp"

namespace engine {
    // void SimulationState::on_init(std::shared_ptr<Scene> scene) {
    //     scene_ = std::move(scene);
    //     if (scene_) {
    //         scene_->on_init();
    //     }
    // }
    //
    // void SimulationState::on_shutdown() {
    //     stop();
    //     scene_->on_shutdown();
    //     scene_.reset();
    // }

    void SceneManager::on_attach(std::shared_ptr<Scene> scene) {
        if (scene_) {
            scene_->on_shutdown();
        }
        scene_ = std::move(scene);
        if (scene_) {
            scene_->on_init();
        }
    }

    void SceneManager::on_detach() {
        on_stop();
        if (scene_) {
            scene_->on_shutdown();
            scene_.reset();
        }
    }

    void SceneManager::on_update(float delta_time) {
        simulation_state_.on_update(delta_time);
        if (scene_state() == SceneState::Play && simulation_scene_) {
            simulation_scene_->on_update(delta_time);
        }
    }

    void SceneManager::on_play() {
        if (scene_state() == SceneState::Pause) {
            simulation_state_.play();
            CORE_TRACE("Resumed scene simulation");
            return;
        }

        simulation_state_.play();
        CORE_TRACE("Started scene simulation");

        simulation_scene_ = Scene::copy(scene_);
        if (simulation_scene_) {
            simulation_scene_->on_init();
        }
    }

    void SceneManager::on_pause() {
        if (scene_state() == SceneState::Play) {
            simulation_state_.pause();
            CORE_TRACE("Paused scene simulation");
        }
    }

    void SceneManager::on_stop() {
        if (scene_state() == SceneState::Edit) return;

        simulation_state_.stop();
        if (simulation_scene_) {
            simulation_scene_->on_shutdown();
            simulation_scene_.reset();
        }
        CORE_TRACE("Stopped scene simulation");
    }

    std::shared_ptr<Scene> SceneManager::active_scene() const {
        return scene_state() == SceneState::Edit ? scene_ : simulation_scene_;
    }

    SceneState SceneManager::scene_state() const { return simulation_state_.state(); }

    float SceneManager::simulation_time() const { return simulation_state_.time(); }
}
