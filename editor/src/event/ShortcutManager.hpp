//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>

#include "EditorActions.hpp"
#include "input/KeyCodes.hpp"

namespace editor {
    using namespace engine;

    struct KeyBinding {
        std::string_view name;
        KeyCode key;
        KeyCode modifiers; // Ctrl, Shift, Alt
    };

    class ShortcutManager {
    public:
        ShortcutManager() = default;
        ~ShortcutManager() = default;

        void bind(EditorAction action, std::string_view name, KeyCode key, KeyCode modifiers = 0);
        [[nodiscard]] bool triggered(EditorAction action) const;
        [[nodiscard]] std::optional<EditorAction> find_action(KeyCode key, KeyCode active_modifier) const;

        [[nodiscard]] std::string_view shortcut_text(EditorAction action) const;

    private:
        std::unordered_map<EditorAction, KeyBinding> bindings_;
    };
}
