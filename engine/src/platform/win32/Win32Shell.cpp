//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "platform/win32/Win32Shell.hpp"

#ifdef _WIN32
#include <windows.h>

#include <shellapi.h>

namespace win32 {
    void open_url(const std::string& url) {
        ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
}
#endif
