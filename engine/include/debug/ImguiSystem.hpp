//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    class ImguiSystem {
    public:
        static void on_attach(void* window, const char* glsl_version = "#version 450");
        static void on_detach();

        static void begin_frame();
        static void end_frame();
    };
}
