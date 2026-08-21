//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/UUID.hpp"

namespace engine {
    static std::random_device random_device;
    static std::mt19937_64 engine(random_device());
    static std::uniform_int_distribution<uint64_t> uniform_distribution;

    UUID::UUID() : uuid_(uniform_distribution(engine)) {}

    UUID::UUID(const uint64_t uuid) : uuid_(uuid) {}
}
