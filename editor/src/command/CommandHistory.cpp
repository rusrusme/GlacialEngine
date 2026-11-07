//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "CommandHistory.hpp"

namespace editor {
    CommandHistory::CommandHistory(size_t capacity) : capacity_(capacity) {}

    void CommandHistory::begin_transaction(std::string_view name) {
        if (nesting_level_ == 0) {
            active_transaction_ = std::make_unique<CommandBatch>(name);
        }
        ++nesting_level_;
    }

    void CommandHistory::record(std::unique_ptr<Command> cmd) {
        if (!cmd) return;

        cmd->execute();

        if (nesting_level_ > 0) {
            active_transaction_->add(std::move(cmd));
        }
        else {
            begin_transaction(cmd->name());
            active_transaction_->add(std::move(cmd));
            end_transaction();
        }
    }

    void CommandHistory::end_transaction() {
        if (nesting_level_ == 0) return;

        --nesting_level_;
        if (nesting_level_ > 0) return;

        if (!active_transaction_ || active_transaction_->empty()) {
            active_transaction_.reset();
            return;
        }

        commit(std::move(active_transaction_));
    }

    void CommandHistory::cancel_transaction() {
        if (nesting_level_ == 0 || !active_transaction_) {
            return;
        }

        // Roll back any sub-commands executed within the unfinished transaction
        active_transaction_->undo();
        active_transaction_.reset();
        nesting_level_ = 0;
    }

    bool CommandHistory::can_undo() const noexcept {
        return current_index_ > 0 && nesting_level_ == 0;
    }

    bool CommandHistory::can_redo() const noexcept {
        return current_index_ < history_.size() && nesting_level_ == 0;
    }

    void CommandHistory::undo() {
        if (!can_undo()) return;

        --current_index_;
        history_[current_index_]->undo();
    }

    void CommandHistory::redo() {
        if (!can_redo()) return;

        history_[current_index_]->execute();
        ++current_index_;
    }

    void CommandHistory::set_clean() noexcept {
        clean_index_ = current_index_;
    }

    bool CommandHistory::dirty() const noexcept {
        return clean_index_ != current_index_;
    }

    void CommandHistory::clear() {
        history_.clear();
        active_transaction_.reset();
        nesting_level_ = 0;
        current_index_ = 0;
        clean_index_ = 0;
    }

    void CommandHistory::commit(std::unique_ptr<CommandBatch> transaction) {
        if (current_index_ > 0 && history_[current_index_ - 1]->merge(*transaction)) {
            return;
        }

        if (current_index_ < history_.size()) {
            history_.erase(history_.begin() + static_cast<std::ptrdiff_t>(current_index_), history_.end());
            if (clean_index_ && *clean_index_ >= current_index_) {
                clean_index_ = std::nullopt;
            }
        }

        if (history_.size() >= capacity_) {
            history_.pop_front();
            if (clean_index_) {
                if (*clean_index_ == 0) {
                    clean_index_ = std::nullopt; // Saved checkpoint evicted from memory
                } else {
                    --*clean_index_;
                }
            }
            --current_index_;
        }

        history_.push_back(std::move(transaction));
        current_index_ = history_.size();
    }
}
