//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace editor {
    class Command;

    class CommandHistory {
    public:
        explicit CommandHistory(size_t max_capacity = 100)
            : max_capacity_(max_capacity) {}

        template<typename T, typename... Args>
        void execute_command(Args&&... args) {
            std::unique_ptr<Command> cmd = std::make_unique<T>(std::forward<Args>(args)...);
            execute_command(std::move(cmd));
        }

        void execute_command(std::unique_ptr<Command> cmd);

        [[nodiscard]] bool can_undo() const { return cursor_ > 0; }
        [[nodiscard]] bool can_redo() const { return cursor_ < commands_.size(); }

        void undo();
        void redo();
        void clear();

    private:
        std::vector<std::unique_ptr<Command>> commands_;
        size_t cursor_ = 0;
        size_t max_capacity_ = 100;
    };
}
