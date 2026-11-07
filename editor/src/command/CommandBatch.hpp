//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Command.hpp"

namespace editor {
    class CommandBatch : public Command {
    public:
        explicit CommandBatch(std::string_view name) : name_(name) {}
        ~CommandBatch() override = default;

        CommandBatch(const CommandBatch& other);
        CommandBatch& operator=(const CommandBatch&) = delete;
        CommandBatch(CommandBatch&&) noexcept = default;
        CommandBatch& operator=(CommandBatch&&) noexcept = default;

        void add(std::unique_ptr<Command> cmd);

        void execute(SystemContext& context) override;
        void undo(SystemContext& context) override;

        bool merge(const Command& next) override;
        [[nodiscard]] std::unique_ptr<Command> clone() const override;

        [[nodiscard]] std::string_view name() const override { return name_; }
        [[nodiscard]] bool empty() const noexcept { return commands_.empty(); }

    private:
        std::string name_;
        std::vector<std::unique_ptr<Command>> commands_;
    };
}
