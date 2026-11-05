//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "EditorActions.hpp"
#include "input/KeyCodes.hpp"

namespace editor {
    using namespace engine;

    enum class ShortcutContext : uint8_t {
        Global = 0,
        Viewport,
        Hierarchy,
        Inspector,
        AssetBrowser,
        Console,
    };

    enum KeyModifier : uint8_t {
        None  = 0,
        Ctrl  = 1 << 0, // 1
        Shift = 1 << 1, // 2
        Alt   = 1 << 2, // 4
        Super = 1 << 3  // 8
    };

    inline KeyModifier operator|(KeyModifier a, KeyModifier b) {
        return static_cast<KeyModifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline KeyModifier operator|=(KeyModifier& a, KeyModifier b) {
        a = a | b;
        return a;
    }

    struct KeyBinding {
        EditorAction action;
        std::string_view name;
        KeyCode key;
        KeyModifier modifiers = None; // Ctrl, Shift, Alt, Super
        ShortcutContext context = ShortcutContext::Global;
    };

    class ShortcutManager {
    public:
        ShortcutManager() = delete;
        ~ShortcutManager() = delete;

        static void bind(EditorAction action,
                         std::string_view name,
                         KeyCode key,
                         KeyModifier modifiers = None,
                         ShortcutContext context = ShortcutContext::Global);
        static std::optional<EditorAction> find_action(KeyCode key,
                                                       KeyModifier active_modifier = None,
                                                       ShortcutContext current_context = ShortcutContext::Global);

        static std::string_view shortcut_text(EditorAction action);

        static void set_active_context(ShortcutContext context) { s_active_context = context; }
        [[nodiscard]] static ShortcutContext active_context() { return s_active_context; }

    private:
        inline static auto s_active_context = ShortcutContext::Global;
        inline static std::vector<KeyBinding> s_bindings;
    };
}
