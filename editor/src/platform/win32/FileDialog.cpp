//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "FileDialog.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>

namespace win32 {
    std::string open_file(const char* filter) {
        OPENFILENAMEA ofn;
        char sz_file[260] = {};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = sizeof(sz_file);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            return {ofn.lpstrFile};
        }
        return "";
    }

    std::string save_file(const char* filter, const char* extension) {
        OPENFILENAMEA ofn;
        char sz_file[260] = {};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = sizeof(sz_file);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.lpstrDefExt = extension;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        if (GetSaveFileNameA(&ofn) == TRUE) {
            return {ofn.lpstrFile};
        }
        return "";
    }
}
#endif
