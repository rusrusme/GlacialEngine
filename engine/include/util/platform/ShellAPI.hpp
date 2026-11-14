//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

namespace engine::platform {
    class ShellAPI {
    public:
        static void open_url(const std::string& url);
    };
}
