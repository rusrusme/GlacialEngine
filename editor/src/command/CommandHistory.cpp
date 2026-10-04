//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "command/CommandHistory.hpp"

namespace editor {
    void CommandHistory::execute_command(std::unique_ptr<Command> cmd) {
        if (!cmd) return;
        cmd->execute();
        if (cursor_ > 0 && commands_[cursor_ - 1]->merge_with(cmd.get())) {
            return;
        }
        if (cursor_ < commands_.size()) {
            commands_.erase(commands_.begin() + static_cast<ptrdiff_t>(cursor_), commands_.end());
        }
        commands_.push_back(std::move(cmd));
        if (commands_.size() > max_capacity_) {
            commands_.erase(commands_.begin());
        }
        else {
            cursor_++;
        }
    }

    void CommandHistory::undo() {
        if (!can_undo()) return;
        cursor_--;
        commands_[cursor_]->undo();
    }

    void CommandHistory::redo() {
        if (!can_redo()) return;
        commands_[cursor_]->execute();
        cursor_++;
    }

    void CommandHistory::clear() {
        commands_.clear();
        cursor_ = 0;
    }
}
