//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string_view>

namespace editor {
    class Command {
    public:
        virtual ~Command() = default;
        virtual void execute() = 0;
        virtual void undo() = 0;

        virtual bool merge_with(const Command* next) { return false; }
        [[nodiscard]] virtual std::string_view get_name() const = 0;
    };
}
