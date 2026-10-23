//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "project/scene/SceneManager.hpp"

#include "core/Log.hpp"
#include "core/SystemContext.hpp"
#include "project/scene/SceneSerializer.hpp"

namespace engine {
    void SceneManager::on_attach(std::shared_ptr<Scene> scene) {
        if (scene_) {
            scene_->on_shutdown();
        }
        scene_ = std::move(scene);
        if (scene_) {
            scene_->on_init();
        }
    }

    void SceneManager::shutdown() {
        on_stop();
        if (scene_) {
            scene_->on_shutdown();
            scene_.reset();
        }
    }

    void SceneManager::save(const std::string& filepath, const SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;
        SceneSerializer serializer{scene};
        serializer.serialize(filepath, context.assets);
    }

    bool SceneManager::load(const std::string& filepath, const SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return false;
        SceneSerializer serializer{scene};
        if (!serializer.deserialize(filepath, context)) {
            return false;
        }
        return true;
    }

    std::shared_ptr<Scene> SceneManager::create(const std::string& name) {
        const auto scene = std::make_shared<Scene>(name);
        on_attach(scene);
        return scene_;
    }

    void SceneManager::on_update(float delta_time) {
        simulation_state_.on_update(delta_time);
        if (scene_state() == SceneState::Play && simulation_scene_) {
            simulation_scene_->on_update(delta_time);
        }
    }

    void SceneManager::on_event(Event& event) {
        if (scene_state() == SceneState::Play && simulation_scene_) {
            simulation_scene_->on_event(event);
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
