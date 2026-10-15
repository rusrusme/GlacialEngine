//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#ifdef ENABLE_IMGUI

namespace engine {
    enum class GraphicsFramework;

    namespace debug {
        bool init_platform_backend(void* window_handle, GraphicsFramework framework);
        void shutdown_platform_backend(GraphicsFramework framework);
        void new_frame_platform_backend(GraphicsFramework framework);
    }
}

#endif