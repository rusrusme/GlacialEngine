//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <functional>

#include "core/Config.hpp"
#include "core/WindowAPI.hpp"

#include "event/WindowEvents.hpp"

namespace engine {
    class Window {
    public:
        using EventCallbackFunc = std::function<void(Event&)>;

        Window();
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void set_event_callback(const EventCallbackFunc& callback);
        void set_vsync(bool enabled);
        [[nodiscard]] bool is_vsync() const { return setting::video::IS_VSYNC; }
        void set_title(const std::string& title);

        [[nodiscard]] float get_monitor_dpi_scale() const;
        [[nodiscard]] float get_aspect_ratio() const { return data_.height > 0 ? static_cast<float>(data_.width) / static_cast<float>(data_.height) : 1.0f; }
        [[nodiscard]] uint32_t get_width() const { return data_.width; }
        [[nodiscard]] uint32_t get_height() const { return data_.height; }

        /** @brief Returns the raw window reference */
        [[nodiscard]] void* native() const { return window_handle_; }
        [[nodiscard]] bool is_valid() const { return window_handle_ != nullptr; }

        void save_state() const;

    private:
        WindowNativeData data_;

        void update_dimensions();

        void* window_handle_ = nullptr;
    };
}
