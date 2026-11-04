//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ShortcutManager.hpp"

#include "input/Input.hpp"

namespace editor {
    void ShortcutManager::bind(EditorAction action, std::string_view name, KeyCode key, KeyCode modifiers) {
        bindings_[action] = {.name = name, .key = key, .modifiers = modifiers};
    }

    // TODO needs work
    bool ShortcutManager::triggered(EditorAction action) const {
        if (Input::keyboard_captured()) {
            return false;
        }

        auto it = bindings_.find(action);
        if (it == bindings_.end()) return false;

        const KeyBinding& binding = it->second;

        bool key_pressed = Input::key_pressed_once(binding.key);

        KeyCode active_mods = 0;
        if (Input::key_pressed(Key::LEFT_CONTROL)) active_mods |= Key::LEFT_CONTROL;
        if (Input::key_pressed(Key::LEFT_SHIFT)) active_mods |= Key::LEFT_SHIFT;
        if (Input::key_pressed(Key::LEFT_ALT)) active_mods |= Key::LEFT_ALT;

        bool mods_match = active_mods == binding.modifiers;

        return key_pressed && mods_match;
    }

    std::optional<EditorAction> ShortcutManager::find_action(KeyCode key, KeyCode active_modifier) const {
        for (const auto& [action, binding] : bindings_) {
            if (binding.key == key) {
                if (binding.modifiers == active_modifier) {
                    return action;
                }
            }
        }
        return std::nullopt;
    }

    std::string_view ShortcutManager::shortcut_text(EditorAction action) const {
        auto it = bindings_.find(action);
        if (it != bindings_.end()) {
            return it->second.name;
        }
        return "";
    }
}
