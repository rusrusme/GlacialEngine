//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace editor {
    class Console {
    public:
        Console() = default;

        #ifdef ENABLE_IMGUI
        void render_imgui();
        #endif
    };
}
