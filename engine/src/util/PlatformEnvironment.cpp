//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "util/PlatformEnvironment.hpp"

#if defined(_WIN32)
#include <windows.h>

#include <array>
#include <memory>
#endif

#include "core/Log.hpp"

namespace engine::platform {
    void init_build_environment() {
        #if defined(_WIN32)
        if (GetEnvironmentVariableA("VSCMD_VER", nullptr, 0) > 0) {
            CORE_INFO("[PlatformEnvironment] MSVC developer environment already present");
            return;
        }

        CORE_INFO("[PlatformEnvironment] Initializing MSVC build environment...");

        constexpr auto cmd = "cmd.exe /s /c \""
            "for /f \"usebackq tokens=*\" %i in (`\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\" "
            "-latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do ("
            "call \"%i\\VC\\Auxiliary\\Build\\vcvars64.bat\" >nul 2>&1 && set"
            ")\"";

        std::array<char, 512> buffer{};
        std::string result;

        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
        if (!pipe) {
            CORE_WARN("[PlatformEnvironment] Failed to execute vswhere/vcvars probe");
            return;
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        size_t start = 0;
        size_t count = 0;
        while (start < result.size()) {
            size_t end = result.find('\n', start);
            if (end == std::string::npos) end = result.size();

            std::string line = result.substr(start, end - start);
            if (!line.empty() && line.back() == '\r') line.pop_back();

            if (size_t eq = line.find('='); eq != std::string::npos && eq > 0) {
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                SetEnvironmentVariableA(key.c_str(), val.c_str());
                ++count;
            }
            start = end + 1;
        }

        CORE_INFO("[PlatformEnvironment] Successfully imported {} environment variables", count);
        #elif defined(__linux__)
        if (std::system("which cmake > /dev/null 2>&1") != 0) {
            CORE_WARN("[PlatformEnvironment] 'cmake' not found in PATH! Script hot-reload may fail.");
        }
        else {
            CORE_INFO("[PlatformEnvironment] Linux build environment verified.");
        }
        #endif
    }
}
