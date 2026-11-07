//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <string_view>

#include "Command.hpp"
#include "CommandBatch.hpp"

namespace editor {
    class CommandHistory {
    public:
        explicit CommandHistory(size_t capacity = 100);
        ~CommandHistory() = default;

        CommandHistory(const CommandHistory&) = delete;
        CommandHistory& operator=(const CommandHistory&) = delete;
        CommandHistory(CommandHistory&&) noexcept = default;
        CommandHistory& operator=(CommandHistory&&) noexcept = default;

        void begin_transaction(std::string_view name);
        void record(std::unique_ptr<Command> cmd);
        void end_transaction();
        void cancel_transaction();

        [[nodiscard]] bool can_undo() const noexcept;
        [[nodiscard]] bool can_redo() const noexcept;
        void undo();
        void redo();

        void set_clean() noexcept;
        [[nodiscard]] bool dirty() const noexcept;

        void clear();
        [[nodiscard]] size_t size() const noexcept { return history_.size(); }
        [[nodiscard]] size_t current_index() const noexcept { return current_index_; }
        [[nodiscard]] bool in_transaction() const noexcept { return nesting_level_ > 0; }

    private:
        void commit(std::unique_ptr<CommandBatch> transaction);

        std::deque<std::unique_ptr<CommandBatch>> history_;
        std::unique_ptr<CommandBatch> active_transaction_{nullptr};
        size_t current_index_{0};
        size_t capacity_{100};
        size_t nesting_level_{0};
        std::optional<size_t> clean_index_{0};
    };
}
