//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <concepts>

namespace engine {
    class Event;
    class Scene;

    // Compile-time System Concepts
    template<typename T>
    concept HasValidOnUpdate = requires(Scene& s, float dt) {
        { T::on_update(s, dt) } -> std::same_as<void>;
    };

    template<typename T>
    concept HasValidOnEvent = requires(Scene& s, Event& e) {
        { T::on_event(s, e) } -> std::same_as<void>;
    };

    // Diagnostics for common mistakes
    template<typename T>
    concept HasNonStaticOnUpdate = requires(T& instance, Scene& s, float dt) {
        { instance.on_update(s, dt) } -> std::same_as<void>;
    } && !HasValidOnUpdate<T>;

    template<typename T>
    concept HasNonStaticOnEvent = requires(T& instance, Scene& s, Event& e) {
        { instance.on_event(s, e) } -> std::same_as<void>;
    } && !HasValidOnEvent<T>;

    template<typename T>
    concept HasWrongUpdateSignature = (
        requires(float dt) { T::on_update(dt); } ||
        requires(Scene& s) { T::on_update(s); }
    ) && !HasValidOnUpdate<T>;

    struct SceneSystemEntry {
        void (*on_update)(Scene&, float) = nullptr;
        void (*on_event)(Scene&, Event&) = nullptr;
    };
}
