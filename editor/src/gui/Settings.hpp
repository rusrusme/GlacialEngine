//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace editor {
    class Settings {
    public:
        Settings() = default;
        ~Settings() { close(); };

        void open() { is_open_ = true; }
        void close() { is_open_ = false; }
        void toggle() { is_open_ = !is_open_; }
        [[nodiscard]] bool is_open() const { return is_open_; }

        #ifdef ENABLE_IMGUI
        void ui_scale();
        void render_imgui(const engine::SystemContext& context);
        #endif

    private:
        bool is_open_ = false;
        float ui_scale_ = 1.0f;
    };
}
