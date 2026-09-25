//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "platform/FileDialogAPI.hpp"

#if defined(_WIN32)
#include "platform/win32/Win32FileDialog.hpp"
#elif defined(__linux__)
#include "platform/linux/LinuxFileDialog.hpp"
#endif

namespace engine::platform {
    std::string FileDialogAPI::open_file(const char* filter) {
        #if defined(_WIN32)
        return win32::open_file(filter);
        #elif defined(__linux__)
        // return linux_platform::open_file(filter);
        return "";
        #else
        return "";
        #endif
    }

    std::string FileDialogAPI::save_file(const char* filter, const char* extension) {
        #if defined(_WIN32)
        return win32::save_file(filter, extension);
        #elif defined(__linux__)
        // return linux_platform::save_file(filter, extension);
        return "";
        #else
        return "";
        #endif
    }
}
