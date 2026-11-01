//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <algorithm>
#include <vector>

namespace glaze::runtime {
    template<typename T, typename I>
    inline void list_remove_at(std::vector<T>& vec, I index) {
        vec.erase(vec.begin() + static_cast<std::ptrdiff_t>(index));
    }

    template<typename T, typename V>
    inline void list_remove(std::vector<T>& vec, const V& val) {
        std::erase(vec, val);
    }

    template<typename T, typename V>
    inline bool list_has(const std::vector<T>& vec, const V& val) {
        return std::find(vec.begin(), vec.end(), val) != vec.end();
    }
}