//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#ifndef MINIAUDIO_IMPLEMENTATION
#define MINIAUDIO_IMPLEMENTATION
#endif
#include "audio/AudioSystem.hpp"

#include "asset/AssetManager.hpp"
#include "core/Log.hpp"
#include "core/SystemContext.hpp"
#include "project/scene/Scene.hpp"
#include "project/scene/SceneManager.hpp"

namespace engine {
    void AudioSystem::on_attach() {
        ma_result result = ma_engine_init(nullptr, &m_engine);
        if (result != MA_SUCCESS) {
            CORE_ERROR("Failed to initialize audio engine");
            m_initialized = false;
            return;
        }
        m_initialized = true;
        CORE_INFO("Audio system initialized successfully");
    }

    void AudioSystem::on_detach(SystemContext& context) {
        if (!m_initialized) return;

        // ECS sounds
        if (auto scene = context.scenes.active_scene()) {
            auto view = scene->view<AudioSourceComponent>();
            for (auto entity : view) {
                auto& source = view.get<AudioSourceComponent>(entity);
                free_component_sound(source);
            }
        }

        // One-shot sounds
        for (auto* sound : m_one_shot_sounds) {
            ma_sound_uninit(sound);
            delete sound;
        }
        m_one_shot_sounds.clear();

        ma_engine_uninit(&m_engine);
        m_initialized = false;
    }

    void AudioSystem::on_update(SystemContext& context, float delta_time) {
        if (!m_initialized) return;

        if (!context.scenes.active_scene()) return;

        update_listener(context);
        update_sources(context);
        cleanup_finished_sounds();
    }

    void AudioSystem::play_source(SystemContext& context, AudioSourceComponent& source) {
        if (!source.runtime_sound_ptr && !init_component_sound(context, source)) {
            return;
        }

        auto* sound = static_cast<ma_sound*>(source.runtime_sound_ptr);
        ma_sound_start(sound);
        source.is_playing = true;
    }

    void AudioSystem::stop_source(AudioSourceComponent& source) {
        if (!source.runtime_sound_ptr) return;

        auto* sound = static_cast<ma_sound*>(source.runtime_sound_ptr);
        ma_sound_stop(sound);
        ma_sound_seek_to_pcm_frame(sound, 0); // Rewind to start
        source.is_playing = false;
    }

    void AudioSystem::pause_source(AudioSourceComponent& source) {
        if (!source.runtime_sound_ptr) return;

        auto* sound = static_cast<ma_sound*>(source.runtime_sound_ptr);
        ma_sound_stop(sound); // Stops playback at current frame position
        source.is_playing = false;
    }

    bool AudioSystem::is_source_playing(const AudioSourceComponent& source) const {
        if (!source.runtime_sound_ptr) return false;
        auto* sound = static_cast<const ma_sound*>(source.runtime_sound_ptr);
        return ma_sound_is_playing(sound);
    }

    bool AudioSystem::play_sound(const std::string& filepath, float volume, float pitch) {
        if (!m_initialized) return false;

        auto* sound = new ma_sound();
        // MA_SOUND_FLAG_ASYNC prevents audio decoding from stalling the frame
        ma_uint32 flags = MA_SOUND_FLAG_ASYNC;
        ma_result result = ma_sound_init_from_file(&m_engine, filepath.c_str(), flags, nullptr, nullptr, sound);

        if (result != MA_SUCCESS) {
            delete sound;
            return false;
        }

        ma_sound_set_volume(sound, volume);
        ma_sound_set_pitch(sound, pitch);
        ma_sound_start(sound);

        m_one_shot_sounds.push_back(sound);
        return true;
    }

    void AudioSystem::set_master_volume(float volume) {
        if (!m_initialized) return;
        ma_engine_set_volume(&m_engine, volume);
    }

    void AudioSystem::set_listener_position(float x, float y, float z) {
        if (!m_initialized) return;
        ma_engine_listener_set_position(&m_engine, 0, x, y, z);
    }

    void AudioSystem::update_listener(SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;

        const auto view = scene->view<TransformComponent, AudioListenerComponent>(entt::exclude<DisabledComponent>);
        for (const auto& [entity, transform, listener] : view.each()) {
            if (!listener.is_primary) continue;

            const glm::vec3 pos = transform.position;
            const glm::vec3 forward = transform.get_forward();
            const glm::vec3 up = transform.get_up();

            ma_engine_listener_set_position(&m_engine, 0, pos.x, pos.y, pos.z);
            ma_engine_listener_set_direction(&m_engine, 0, forward.x, forward.y, forward.z);
            ma_engine_listener_set_world_up(&m_engine, 0, up.x, up.y, up.z);

            break;
        }
    }

    void AudioSystem::update_sources(SystemContext& context) {
        const auto scene = context.scenes.active_scene();
        if (!scene) return;

        const auto view = scene->view<TransformComponent, AudioSourceComponent>(entt::exclude<DisabledComponent>);
        for (const auto& [entity, transform, source] : view.each()) {
            if (source.runtime_sound_ptr) {
                auto* sound = static_cast<ma_sound*>(source.runtime_sound_ptr);

                ma_sound_set_volume(sound, source.volume);
                ma_sound_set_pitch(sound, source.pitch);
                ma_sound_set_looping(sound, source.loop);

                ma_sound_set_spatialization_enabled(sound, source.is_spatial ? MA_TRUE : MA_FALSE);

                if (source.is_spatial) {
                    ma_sound_set_position(sound, transform.position.x, transform.position.y, transform.position.z);
                }
                else {
                    ma_sound_set_position(sound, 0.0f, 0.0f, 0.0f);
                }

                source.is_playing = ma_sound_is_playing(sound);
            }
        }
    }

    void AudioSystem::cleanup_finished_sounds() {
        for (auto it = m_one_shot_sounds.begin(); it != m_one_shot_sounds.end();) {
            if (ma_sound_at_end(*it)) {
                ma_sound_uninit(*it);
                delete *it;
                it = m_one_shot_sounds.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    bool AudioSystem::init_component_sound(SystemContext& context, AudioSourceComponent& source) {
        if (!m_initialized || source.filepath.empty()) return false;

        std::string path_to_load = source.filepath;

        // Resolve handle from AssetManager metadata if valid
        if (source.sound_handle != 0) {
            const auto& meta = context.assets.metadata(source.sound_handle);
            if (meta.valid()) {
                path_to_load = meta.filepath.string();
            }
        }
        if (path_to_load.empty()) return false;

        if (source.runtime_sound_ptr) {
            free_component_sound(source);
        }

        auto* sound = new ma_sound();
        ma_uint32 flags = source.is_spatial ? MA_SOUND_FLAG_ASYNC : (MA_SOUND_FLAG_ASYNC);
        if (!source.is_spatial) {
            flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;
        }

        ma_result result = ma_sound_init_from_file(&m_engine, path_to_load.c_str(), flags, nullptr, nullptr, sound);
        if (result != MA_SUCCESS) {
            CORE_ERROR("Failed to load audio file: {0}", path_to_load);
            delete sound;
            return false;
        }

        ma_sound_set_volume(sound, source.volume);
        ma_sound_set_pitch(sound, source.pitch);
        ma_sound_set_looping(sound, source.loop);
        ma_sound_set_min_distance(sound, source.min_distance);
        ma_sound_set_max_distance(sound, source.max_distance);

        ma_sound_set_spatialization_enabled(sound, source.is_spatial ? MA_TRUE : MA_FALSE);

        source.runtime_sound_ptr = sound;
        return true;
    }

    void AudioSystem::free_component_sound(AudioSourceComponent& source) {
        if (source.runtime_sound_ptr) {
            auto* sound = static_cast<ma_sound*>(source.runtime_sound_ptr);
            ma_sound_uninit(sound);
            delete sound;
            source.runtime_sound_ptr = nullptr;
            source.is_playing = false;
        }
    }
}
