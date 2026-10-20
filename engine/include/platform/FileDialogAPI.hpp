//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>

namespace engine::platform {
    struct FileDialogFilter {
        static constexpr auto AUDIO = "Audio Files (*.wav;*.mp3;*.ogg;*.flac)\0*.wav;*.mp3;*.ogg;*.flac\0All Files (*.*)\0*.*\0";
        static constexpr auto IMAGE = "Image Files (*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.dds;*.hdr)\0*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.dds;*.hdr\0All Files (*.*)\0*.*\0";
        static constexpr auto SCENE = "Scene Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0";
        static constexpr auto PROJECT = "Glacial Project (*.glacial)\0*.glacial\0All Files (*.*)\0*.*\0";
        static constexpr auto ALL = "All Files (*.*)\0*.*\0";
    };

    class FileDialogAPI {
    public:
        static std::string open_file(const char* filter = FileDialogFilter::ALL);
        static std::string save_file(const char* filter = FileDialogFilter::ALL, const char* extension = nullptr);
    };
}
