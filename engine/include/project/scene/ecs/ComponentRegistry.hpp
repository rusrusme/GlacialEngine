//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <ranges>
#include <unordered_map>
#include <entt/entt.hpp>

namespace engine {
    /** @brief Do not use this as a user. Public due to templates */
    class ComponentRegistry {
    public:
        using CopyFunction = void(*)(entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map);
        using CopierMap = std::unordered_map<entt::id_type, CopyFunction>;

        static CopierMap& get_copiers() {
            static CopierMap local_copiers;
            return s_host_copiers ? *s_host_copiers : local_copiers;
        }

        static void set_host_copiers(CopierMap* host_copiers) {
            s_host_copiers = host_copiers;
        }

        template<typename T>
        static void register_type() {
            auto type_id = entt::type_hash<T>::value();
            get_copiers()[type_id] = [](entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map) {
                auto view = src.view<T>();
                for (auto it = view.rbegin(); it != view.rend(); ++it) {
                    auto src_entity = *it;
                    if (auto map_it = map.find(src_entity); map_it != map.end()) {
                        dst.emplace_or_replace<T>(map_it->second, view.template get<T>(src_entity));
                    }
                }
            };
        }

        static void copy_all_components(entt::registry& src, entt::registry& dst, const std::unordered_map<entt::entity, entt::entity>& map) {
            for (auto& copy_fn : get_copiers() | std::views::values) {
                copy_fn(src, dst, map);
            }
        }

        static void clear() {
            get_copiers().clear();
        }

    private:
        inline static CopierMap* s_host_copiers = nullptr;
    };
}
