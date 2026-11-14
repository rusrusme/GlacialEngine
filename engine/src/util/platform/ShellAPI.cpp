//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "util/platform/ShellAPI.hpp"

#if defined(_WIN32)
#include "win32/Win32Shell.hpp"
#elif defined(__linux__)
#include "linux/LinuxShell.hpp"
#endif

namespace engine::platform {
    void ShellAPI::open_url(const std::string& url) {
        #if defined(_WIN32)
        win32::open_url(url);
        #elif defined(__linux__)
        // TODO: xdg-open
        #endif
    }
}
