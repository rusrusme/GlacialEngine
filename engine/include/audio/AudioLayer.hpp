//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"

#include <miniaudio.h>
#include <string>

namespace engine {
    struct AudioSourceComponent;
    class Scene;

    class AudioLayer : public Layer {
    public:
        explicit AudioLayer(std::shared_ptr<Scene> scene, std::shared_ptr<AssetManager> asset_manager);

        ~AudioLayer() override = default;

        void on_attach(Renderer& _) override;
        void on_detach() override;
        void on_update(float delta_time) override;
        void on_event(Event& event) override;

        // -----------------------------------
        // Audio source component controls
        // -----------------------------------
        void play_source(AudioSourceComponent& source);
        void stop_source(AudioSourceComponent& source);
        void pause_source(AudioSourceComponent& source);
        [[nodiscard]] bool is_source_playing(const AudioSourceComponent& source) const;

        // -----------------------------------
        // One-shot audio controls
        // -----------------------------------
        bool play_sound(const std::string& filepath, float volume = 1.0f, float pitch = 1.0f);
        void set_master_volume(float volume); // Range: 0.0f to 1.0f (or higher for amplification)
        void set_listener_position(float x, float y, float z);

    private:
        std::shared_ptr<Scene> active_scene_;
        std::shared_ptr<AssetManager> asset_manager_;

        ma_engine m_engine{};
        bool m_initialized{false};

        std::vector<ma_sound*> m_one_shot_sounds;

        // -----------------------------------
        void update_listener();
        void update_sources();
        void cleanup_finished_sounds();

        // -----------------------------------
        bool init_component_sound(AudioSourceComponent& source);
        void free_component_sound(AudioSourceComponent& source);
    };
}
