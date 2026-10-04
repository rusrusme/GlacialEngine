//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace editor {
    class Settings {
    public:
        Settings() = default;
        ~Settings() { close(); }

        void init();
        void save_config() const;

        void open() { is_open_ = true; }
        void close() { is_open_ = false; }
        void toggle() { is_open_ = !is_open_; }
        [[nodiscard]] bool is_open() const { return is_open_; }

        void on_update(const engine::SystemContext& context, float delta_time);

        #ifdef ENABLE_IMGUI
        void apply_ui_scale() const;
        void ui_scale();
        void render_imgui(const engine::SystemContext& context);
        #endif

    private:
        bool is_open_ = false;
        float ui_scale_ = 1.0f;

        bool auto_save_enabled_ = false;
        int auto_save_interval_minutes_ = 5;
        float auto_save_timer_ = 0.0f;
    };
}
