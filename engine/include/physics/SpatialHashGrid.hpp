//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <unordered_map>
#include <vector>
#include <entt/entt.hpp>

#include "physics/AABB.hpp"

namespace engine {
    struct GridKey {
        int x = 0;
        int z = 0;

        bool operator==(const GridKey& other) const {
            return x == other.x && z == other.z;
        }
    };
}

namespace std {
    template <>
    struct hash<engine::GridKey> {
        std::size_t operator()(const engine::GridKey& k) const noexcept {
            return std::hash<int>()(k.x) ^ (std::hash<int>()(k.z) << 1);
        }
    };
}

namespace engine {
    class SpatialHashGrid {
    public:
        explicit SpatialHashGrid(float cell_size = 10.0f);

        void clear() { grid_.clear(); }

        void insert(entt::entity entity, const AABB& world_aabb);

        [[nodiscard]] std::vector<entt::entity> query_range(const AABB& area) const;

    private:
        float cell_size_;
        std::unordered_map<GridKey, std::vector<entt::entity>> grid_;
    };
}