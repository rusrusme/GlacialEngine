//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <unordered_map>
#include "entt/entt.hpp"

namespace engine {
    class ComponentRegistry {
    public:
        using CopyFunction = void(*)(entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map);

    private:
        static auto& get_copiers() {
            static std::unordered_map<entt::id_type, CopyFunction> copiers;
            return copiers;
        }

    public:
        template<typename T>
        static void register_type() {
            // Hot reloading code not possible with this approach
            static const bool registered = []() {
                auto type_id = entt::type_hash<T>::value();
                get_copiers()[type_id] = [](entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map) {
                    auto view = src.view<T>();
                    // Because view iterates reversed
                    for (auto it = view.rbegin(); it != view.rend(); ++it) {
                        auto src_entity = *it;
                        if (auto map_it = map.find(src_entity); map_it != map.end()) {
                            dst.emplace_or_replace<T>(map_it->second, view.template get<T>(src_entity));
                        }
                    }
                };
                return true;
            }();
            (void)registered;
        }

        static void copy_all_components(entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map) {
            for (auto& copy_fn : get_copiers() | std::views::values) {
                copy_fn(src, dst, map);
            }
        }
    };

    // If I ever decide to implement hot reloading DLLs then consider this:
    /*
    #include <atomic>

    static uint32_t& get_epoch() {
        static uint32_t epoch = 1;
        return epoch;
    }

    // Call this whenever you unload/reload a DLL or want to reset metadata!
    static void on_hot_reload() {
        get_copiers().clear();
        get_epoch()++;
    }

    template<typename T>
    static void register_type() {
        // Local static tracks which epoch THIS template instantiation registered for
        static uint32_t registered_epoch = 0;
        const uint32_t current_epoch = get_epoch();

        // Branch predictor will evaluate this to false 99.999% of the time
        if (registered_epoch != current_epoch) {
            registered_epoch = current_epoch;

            auto type_id = entt::type_hash<T>::value();
            get_copiers()[type_id] = [](entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map) {
                auto view = src.view<T>();
                for (auto src_entity : view) {
                    if (auto it = map.find(src_entity); it != map.end()) {
                        dst.emplace_or_replace<T>(it->second, view.template get<T>(src_entity));
                    }
                }
            };
        }
    }
    */
}
