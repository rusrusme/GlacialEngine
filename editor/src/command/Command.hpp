//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <string_view>

#include "core/SystemContext.hpp"

namespace editor {
    using namespace engine;

    class Command {
    public:
        Command() = default;
        virtual ~Command() = default;

        virtual void execute(SystemContext& context) = 0;
        virtual void undo(SystemContext& context) = 0;

        virtual bool merge(const Command& next) { return false; }
        [[nodiscard]] virtual std::unique_ptr<Command> clone() const = 0;

        [[nodiscard]] virtual std::string_view name() const = 0;
    };
}
