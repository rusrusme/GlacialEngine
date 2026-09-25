//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

namespace win32 {
    std::string open_file(const char* filter);
    std::string save_file(const char* filter, const char* extension);
}
