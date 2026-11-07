//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "CommandHistory.hpp"

namespace editor {
    /**
     * A Transaction is a scope guard that bundles multiple executed commands into a single undo/redo step
     * within CommandHistory while ensuring automatic commit on completion and rollback on failure.
     */
    class Transaction {
    public:
        Transaction(std::string_view name, CommandHistory& history)
            : history_(history) {
            history_.begin_transaction(name);
        }

        ~Transaction() {
            if (!dismissed_) {
                history_.end_transaction();
            }
        }

        void cancel(SystemContext& context) {
            if (!dismissed_) {
                history_.cancel_transaction(context);
                dismissed_ = true;
            }
        }

        Transaction(const Transaction&) = delete;
        Transaction& operator=(const Transaction&) = delete;
        Transaction(Transaction&&) = delete;
        Transaction& operator=(Transaction&&) = delete;

    private:
        CommandHistory& history_;
        bool dismissed_{false};
    };
}
