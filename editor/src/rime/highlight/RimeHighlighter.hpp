//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace editor {
    enum class HighlightKind : uint8_t {
        Default = 0,
        Keyword,
        Type,
        Intrinsic,
        Function,
        Number,
        String,
        Operator,
        Punctuation,
        Comment
    };

    struct HighlightSpan {
        uint32_t column_offset{0}; // Byte offset from start of the line
        uint32_t length{0};        // Byte length of token
        HighlightKind kind{HighlightKind::Default};
    };

    struct HighlightedLine {
        uint32_t line_start_offset{0};
        uint32_t line_length{0};
        std::vector<HighlightSpan> spans;
    };

    class RimeHighlighter {
    public:
        [[nodiscard]] static std::vector<HighlightedLine> tokenize(std::string_view source);
        [[nodiscard]] static ImU32 color_for(HighlightKind kind) noexcept;
    };
}
