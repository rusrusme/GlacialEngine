//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <vector>

#include "System.hpp"
#include "event/Event.hpp"

namespace engine {
    class Scene;

    class SystemDispatcher {
    public:
        void on_update(Scene& scene, float delta_time) {
            for (const auto& system : systems_) {
                if (system.on_update) system.on_update(scene, delta_time);
            }
        }

        void on_event(Scene& scene, Event& event) {
            for (const auto& system : systems_) {
                if (system.on_event) {
                    system.on_event(scene, event);
                    if (event.handled) return;
                }
            }
        }

        void clear() { systems_.clear(); }

        // Class-based registration: scene->add_system<MySystem>();
        template<typename T>
        void add_system() {
            static_assert(!HasNonStaticOnUpdate<T>,
                "[Scene::add_system] T::on_update must be declared 'static' -> static void on_update(Scene&, float)");

            static_assert(!HasNonStaticOnEvent<T>,
                "[Scene::add_system] T::on_event must be declared 'static' -> static void on_event(Scene&, Event&)");

            // Check for signature mistakes (e.g., forgot Scene&)
            static_assert(!HasWrongUpdateSignature<T>,
                "[Scene::add_system] Invalid parameter list in T::on_update. Expected signature: static void on_update(Scene&, float)");

            // Must implement at least one hook
            constexpr bool has_update = HasValidOnUpdate<T>;
            constexpr bool has_event  = HasValidOnEvent<T>;

            static_assert(has_update || has_event,
                "[Scene::add_system] Type does not qualify as an engine system. "
                "Must provide at least 'static void on_update(Scene&, float)' or 'static void on_event(Scene&, Event&)'");

            SceneSystemEntry entry{};

            if constexpr (has_update) {
                entry.on_update = &T::on_update;
            }

            if constexpr (has_event) {
                entry.on_event = &T::on_event;
            }

            systems_.push_back(entry);
        }

        // Convenience: scene->add_system(MySystem{});
        template<typename T>
        void add_system(T&&) {
            add_system<std::decay_t<T>>();
        }

        // Direct function-pointer registration for standalone free functions
        void add_system(void(*on_update)(Scene&, float), void(*on_event)(Scene&, Event&) = nullptr) {
            systems_.push_back({.on_update = on_update, .on_event = on_event});
        }

    private:
        std::vector<SceneSystemEntry> systems_;
    };
}
