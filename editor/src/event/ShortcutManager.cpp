//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "ShortcutManager.hpp"

#include "core/Log.hpp"

namespace editor {
    void ShortcutManager::bind(EditorAction action,
                               std::string_view name,
                               KeyCode key,
                               KeyModifier modifiers,
                               ShortcutContext context) {
        for (const auto& existing : s_bindings) {
            if (existing.key == key && existing.modifiers == modifiers) {
                if (existing.context == context) {
                    CLIENT_WARN("Shortcut conflict: '{0}' cannot be bound to key {1}. "
                                "Already claimed by '{2}' in the same scope",
                                name, key, existing.name);
                    return;
                }

                if (existing.context == ShortcutContext::Global || context == ShortcutContext::Global) {
                    CLIENT_TRACE("Shortcut notice: '{0}' overlaps with global shortcut '{1}' on key {2}.",
                                 name, existing.name, key);
                }
            }
        }
        s_bindings.push_back({
            .action    = action,
            .name      = name,
            .key       = key,
            .modifiers = modifiers,
            .context   = context
        });
    }

    std::optional<EditorAction> ShortcutManager::find_action(KeyCode key,
                                                             KeyModifier active_modifier,
                                                             ShortcutContext current_context) {
        const KeyBinding* global_candidate = nullptr;

        for (const auto& binding : s_bindings) {
            if (binding.key != key || binding.modifiers != active_modifier) {
                continue;
            }
            if (binding.context == current_context && current_context != ShortcutContext::Global) {
                return binding.action;
            }
            if (binding.context == ShortcutContext::Global) {
                global_candidate = &binding;
            }
        }
        if (global_candidate) {
            return global_candidate->action;
        }
        return std::nullopt;
    }

    std::string_view ShortcutManager::shortcut_text(EditorAction action) {
        for (const auto& binding : s_bindings) {
            if (binding.action == action) {
                return binding.name;
            }
        }
        return "";
    }
}
