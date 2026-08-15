//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/Frame.hpp"

#include <render/DebugFrame.hpp>

namespace engine {
    void Frame::on_init(GLFWwindow* window) {

        #ifdef ENABLE_IMGUI
        DebugFrame::init(window);
        #endif
    }

    void Frame::on_shutdown() {
        #ifdef ENABLE_IMGUI
        DebugFrame::shutdown();
        #endif
    }

    void Frame::begin_frame() {
        #ifdef ENABLE_IMGUI
        DebugFrame::begin_frame();
        #endif
    }

    void Frame::end_frame() {
        #ifdef ENABLE_IMGUI
        DebugFrame::end_frame();
        #endif
    }
}
