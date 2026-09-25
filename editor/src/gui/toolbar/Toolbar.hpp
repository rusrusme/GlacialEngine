//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <functional>
#include <optional>
#include <string>

#ifdef ENABLE_IMGUI
#include "imgui.h"
#endif

namespace editor {
    struct ToolbarItem {
        std::string id;                                    // Unique identifier for ImGui IDs
        std::string label;                                 // Display name shown during drag-and-drop
        float width = 36.0f;                               // Custom width for this slot (e.g., buttons, separators, dropdowns)
        std::function<void(ImVec2 slot_size)> render_func; // Callback to draw the widget
    };

    struct ToolbarSlot {
        std::string id;
        std::optional<ToolbarItem> item = std::nullopt; // Leer oder belegt
    };
}
