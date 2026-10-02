//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "platform/ShellAPI.hpp"

#if defined(_WIN32)
#include "platform/win32/Win32Shell.hpp"
#elif defined(__linux__)
#include "platform/linux/LinuxShell.hpp"
#endif

namespace engine::platform {
    void ShellAPI::open_url(const std::string& url) {
        #if defined(_WIN32)
        return win32::open_url(url);
        #elif defined(__linux__)
        // return linux_platform::open_file(filter);
        return "";
        #else
        return "";
        #endif
    }
}
