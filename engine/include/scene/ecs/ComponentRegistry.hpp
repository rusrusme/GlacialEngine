//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <functional>
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
            auto type_id = entt::type_hash<T>::value();
            auto& copiers = get_copiers();

            if (copiers.find(type_id) != copiers.end()) return;

            copiers[type_id] = [](entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map) {
                auto view = src.view<T>();
                for (auto src_entity : view) {
                    if (auto it = map.find(src_entity); it != map.end()) {
                        dst.emplace_or_replace<T>(it->second, view.template get<T>(src_entity));
                    }
                }
            };
        }

        static void copy_all_components(entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map) {
            for (auto& copy_fn : get_copiers() | std::views::values) {
                copy_fn(src, dst, map);
            }
        }
    };
}
