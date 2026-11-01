//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>

#include "Token.hpp"

namespace glaze {

    class GLexer {
    public:
        GLexer() = default;

        std::vector<Token> tokenize(std::string_view source_code);

    private:
        void scan_token();
        void identifier();
        void number();
        void string();
        void character();

        bool match(char expected);
        [[nodiscard]] char peek() const;
        [[nodiscard]] char peek_next() const;
        char advance();

        [[nodiscard]] bool is_at_end() const;
        static bool is_digit(char c);
        static bool is_alphabet(char c);
        static bool is_alphabet_numeric(char c);

        void add_token(TokenType type);
        void add_token(TokenType type, TokenLiteral literal);

    private:
        static const std::unordered_map<std::string_view, TokenType> KEYWORDS;

        std::string_view source_code_{};
        std::vector<Token> tokens_{};

        size_t start_{0};
        size_t current_{0};
        uint32_t line_{1};
        uint32_t column_{0};
        uint32_t token_start_line_{1};
        uint32_t token_start_column_{0};
    };
}
