//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <functional>
#include <optional>
#include <string>

#include "ui/UIComponent.hpp"

namespace editor {
    struct ToolbarItem {
        std::string id;                                    // Unique identifier for ImGui IDs
        std::string label;                                 // Display name shown during drag-and-drop
        float width = 36.0f;                               // Custom width for this slot (e.g., buttons, separators, dropdowns)
        std::function<void(ImVec2 slot_size)> render_func; // Callback to draw the widget
    };

    struct ToolbarSlot {
        std::string id;
        std::optional<ToolbarItem> item = std::nullopt;
    };

    class Toolbar : public UIComponent {
    public:
        Toolbar() : UIComponent("Toolbar") {}
        ~Toolbar() override = default;

        void on_init(SystemContext& context) override;
        void on_render_ui(SystemContext& context) override;

        void load_config() override;
        void save_config() override;

    private:
        std::vector<ToolbarSlot> toolbar_slots_;
    };
}
