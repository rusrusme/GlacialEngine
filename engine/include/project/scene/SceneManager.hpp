//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "project/scene/Scene.hpp"
#include "project/scene/SceneState.hpp"
#include "project/scene/SimulationState.hpp"

namespace engine {
    class SceneManager {
    public:
        static inline const std::string DEFAULT_SCENE = "Untitled Scene";

        void on_attach(std::shared_ptr<Scene> scene);
        void shutdown();

        // --- Scene Management ---
        void save(const std::string& filepath, const SystemContext& context);
        bool load(const std::string& filepath, const SystemContext& context);

        std::shared_ptr<Scene> create(const std::string& name = DEFAULT_SCENE);

        void on_update(float delta_time);
        void on_event(Event& event);

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
