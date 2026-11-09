//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "CommandBatch.hpp"

namespace editor {
    CommandBatch::CommandBatch(const CommandBatch& other) : name_(other.name_) {
        commands_.reserve(other.commands_.size());
        for (const auto& cmd : other.commands_) {
            commands_.push_back(cmd->clone());
        }
    }

    void CommandBatch::add(std::unique_ptr<Command> cmd) {
        if (!cmd) return;

        if (!commands_.empty() && commands_.back()->merge(*cmd)) {
            return;
        }
        commands_.push_back(std::move(cmd));
    }

    void CommandBatch::execute(SystemContext& context) {
        for (auto& cmd : commands_) {
            cmd->execute(context);
        }
    }

    void CommandBatch::undo(SystemContext& context) {
        for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
            (*it)->undo(context);
        }
    }

    bool CommandBatch::merge(const Command& next) {
        auto* other = dynamic_cast<const CommandBatch*>(&next);
        if (!other || other->name() != name_) {
            return false;
        }

        // Only merge single-command batches if their underlying commands allow it.
        // Compound transactions (size > 1) represent distinct macro-actions and must not merge.
        if (commands_.size() == 1 && other->commands_.size() == 1) {
            return commands_.front()->merge(*other->commands_.front());
        }
        return false;
    }

    std::unique_ptr<Command> CommandBatch::clone() const {
        return std::make_unique<CommandBatch>(*this);
    }
}
