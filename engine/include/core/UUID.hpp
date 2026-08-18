//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include <cstdint>

namespace engine {
    class UUID {
    public:
        UUID();
        UUID(uint64_t uuid);

        operator uint64_t() const { return uuid_; }

    private:
        uint64_t uuid_;
    };
}
