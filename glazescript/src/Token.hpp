//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <variant>

namespace glaze {
    enum class TokenType {
        // Keywords
        IF, ELSE,
        SWITCH, CASE, DEFAULT,
        LET, VAR,
        FOR,
        DO, WHILE,
        BREAK, CONTINUE,
        RETURN,
        FN,
        STRUCT,
        ENUM,
        LIST,

        // ECS Keywords & Built-in Types
        ENTITY,
        COMPONENT,
        SYSTEM,
        EXCLUDE,
        ON_UPDATE,
        ON_EVENT,
        EVENT,

        // Primitive Types
        VOID,
        BOOL,
        UINT8, INT8,
        UINT32, INT32,
        UINT64, INT64,
        FLOAT, DOUBLE,
        STRING, CHAR,

        // Math Types
        VEC2, VEC3, VEC4,
        MAT2, MAT3, MAT4,
        QUAT,

        // Literals & Identifiers
        IDENTIFIER,
        BOOL_LITERAL,
        INT_LITERAL,
        FLOAT_LITERAL,
        STRING_LITERAL,
        CHAR_LITERAL,

        // Arithmetic
        PLUS, MINUS, STAR, SLASH, PERCENT,
        PLUS_PLUS, MINUS_MINUS,
        PLUS_EQUALS, MINUS_EQUALS, STAR_EQUALS, SLASH_EQUALS, PERCENT_EQUALS,

        // Logic
        AND_AND, OR_OR,

        // Comparisons
        EQUALS, EQUALS_EQUALS, BANG, BANG_EQUALS,
        LESS, LESS_EQUALS, GREATER, GREATER_EQUALS,

        // Blocks
        LPAR, RPAR,         // ()
        LBRACE, RBRACE,     // {}
        LBRACKET, RBRACKET, // []

        // Separators & Control Flow
        COMMA, COLON, SEMICOLON,
        ARROW, // ->

        // Member Access
        DOT,

        // Comments
        HASH,

        END_OF_FILE
    };

    using TokenLiteral = std::variant<std::monostate, bool, int64_t, double, char, std::string>;

    struct Token {
        TokenType type{TokenType::END_OF_FILE};
        std::string lexeme{};
        TokenLiteral literal{std::monostate{}};
        uint32_t line{1};
        uint32_t column{0};

        Token() = default;

        Token(TokenType type, std::string lexeme, TokenLiteral literal, uint32_t line, uint32_t column)
            : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line), column(column) {}
    };

    class GlazeCompilerError : public std::runtime_error {
    public:
        explicit GlazeCompilerError(const std::string& message)
            : std::runtime_error(message) {}

        GlazeCompilerError(const std::string& message, uint32_t line, uint32_t column)
            : std::runtime_error(std::format("{} at [{}:{}]", message, line, column)),
              line_(line), column_(column) {}

        [[nodiscard]] uint32_t line() const noexcept { return line_; }
        [[nodiscard]] uint32_t column() const noexcept { return column_; }

    private:
        uint32_t line_{0};
        uint32_t column_{0};
    };
}
