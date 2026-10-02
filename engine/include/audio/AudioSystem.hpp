//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Layer.hpp"

#include <miniaudio.h>
#include <string>

#include "project/scene/ecs/Components.hpp"

namespace engine {
    class Scene;

    class AudioSystem {
    public:
        AudioSystem() = default;
        ~AudioSystem() = default;

        void on_attach();
        void on_detach(SystemContext& context);
        void on_update(SystemContext& context, float delta_time);

        // -----------------------------------
        // Audio source component controls
        // -----------------------------------
        void play_source(SystemContext& context, AudioSourceComponent& source);
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
        ma_engine m_engine{};
        bool m_initialized{false};

        std::vector<ma_sound*> m_one_shot_sounds;

        // -----------------------------------
        void update_listener(SystemContext& context);
        void update_sources(SystemContext& context);
        void cleanup_finished_sounds();

        // -----------------------------------
        bool init_component_sound(SystemContext& context, AudioSourceComponent& source);
        void free_component_sound(AudioSourceComponent& source);
    };
}
