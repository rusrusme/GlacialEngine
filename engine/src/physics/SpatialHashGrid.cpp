//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "physics/SpatialHashGrid.hpp"

namespace engine {
    SpatialHashGrid::SpatialHashGrid(float cell_size) : cell_size_(cell_size) {}

    void SpatialHashGrid::insert(entt::entity entity, const AABB& world_aabb) {
        int min_x = static_cast<int>(std::floor(world_aabb.min.x / cell_size_));
        int max_x = static_cast<int>(std::floor(world_aabb.max.x / cell_size_));
        int min_z = static_cast<int>(std::floor(world_aabb.min.z / cell_size_));
        int max_z = static_cast<int>(std::floor(world_aabb.max.z / cell_size_));

        for (int x = min_x; x <= max_x; ++x) {
            for (int z = min_z; z <= max_z; ++z) {
                grid_[{x, z}].push_back(entity);
            }
        }
    }

    std::vector<entt::entity> SpatialHashGrid::query_range(const AABB& area) const {
        std::vector<entt::entity> result;

        int min_x = static_cast<int>(std::floor(area.min.x / cell_size_));
        int max_x = static_cast<int>(std::floor(area.max.x / cell_size_));
        int min_z = static_cast<int>(std::floor(area.min.z / cell_size_));
        int max_z = static_cast<int>(std::floor(area.max.z / cell_size_));

        for (int x = min_x; x <= max_x; ++x) {
            for (int z = min_z; z <= max_z; ++z) {
                auto it = grid_.find({x, z});
                if (it != grid_.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
        return result;
    }
}
